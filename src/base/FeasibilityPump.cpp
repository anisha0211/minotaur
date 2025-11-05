//
//     Minotaur -- Feasibility Pump for MINLP (modular version)
//
#include "LinFeasPump.h"
#include "Environment.h"
#include "Engine.h"
#include "Problem.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"
#include "Logger.h"
#include "FeasibilityPump.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace Minotaur;

#define SPEW 1
FeasibilityPump::FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e) : env_(env), p_(p), e_(e), intTol_(1e-6), maxIter_(100), maxTime_(60.0)
{
    logger_ = env_->getLogger();
    timer_ = env_->getNewTimer();
    srand(1);
}


// Step 1: Initialize the FP
bool FeasibilityPump::initialize() {
    logger_->msgStream(LogInfo) << "FP:init: enter initialize(), p_=" << (void*)p_
                           << " e_=" << (void*)e_ << "\n";
    if (!e_) {
      logger_->msgStream(LogError) << "FP:init: engine is NULL\n";
      std::cerr << "FP:init: engine is NULL\n";
      return false;
    }
    e_->load(p_);
    EngineStatus status = e_->solve();
    if (status != ProvenOptimal && status != ProvenLocalOptimal) return false;

    const double* x = e_->getSolution()->getPrimal();
    x_cont_.assign(x, x + p_->getNumVars());
    roundingStep();
    return true;
}

// Step 2: Projection step (solve NLP)
bool FeasibilityPump::projectionStep() {
    logger_->msgStream(LogInfo)
        << "FP:proj: enter projectionStep(), e_=" << (void*)e_ << std::endl;

    if (!e_) {
        logger_->msgStream(LogError) << "FP:proj: NULL engine" << std::endl;
        return false;
    }

    // Clear and reload problem into the NLP engine
    e_->clear();
    e_->load(p_);

    EngineStatus status = e_->solve();
    if (status != ProvenOptimal &&
        status != ProvenLocalOptimal &&
        status != ProvenFailedCQFeas &&
        status != FailedFeas) {
        logger_->msgStream(LogError)
            << "FP:proj: engine solve failed (status = " << status << ")" << std::endl;
        return false;
    }

    // Get solution from engine
    ConstSolutionPtr sol = e_->getSolution();
    if (!sol) {
        logger_->msgStream(LogError)
            << "FP:proj: engine returned NULL solution" << std::endl;
        return false;
    }

    const double* primal = sol->getPrimal();
    if (!primal) {
        logger_->msgStream(LogError)
            << "FP:proj: NULL primal pointer in engine solution" << std::endl;
        return false;
    }

    // Store projected continuous solution
    UInt n = p_->getNumVars();
    x_cont_.assign(primal, primal + n);

    logger_->msgStream(LogInfo)
        << "FP:proj: updated x_cont_ with " << n << " values" << std::endl;

    return true;
}






// Step 3: Rounding step
void FeasibilityPump::roundingStep() {
    logger_->msgStream(LogInfo) << "FP:round: enter roundingStep(), x_cont_.size()=" << x_cont_.size()
                           << " numVars=" << p_->getNumVars() << "\n";
    if (x_cont_.size() < p_->getNumVars()) {
      logger_->msgStream(LogError) << "FP:round: x_cont_ too small\n";
      return;
    }
    x_int_.resize(p_->getNumVars());
    UInt i = 0;
    for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); ++it, ++i) {
        VariablePtr v = *it;
        double xval = x_cont_[i];
        if (v->getType() == Binary || v->getType() == Integer)
            x_int_[i] = floor(xval + 0.5);
        else
            x_int_[i] = xval;
    }
}

// Step 4: Feasibility check
bool FeasibilityPump::isFeasible() const {
    logger_->msgStream(LogInfo) << "FP:isFeas: enter isFeasible(), x_int_.size()=" << x_int_.size() << "\n";

    for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); ++it) {
        VariablePtr v = *it;
        UInt idx = v->getIndex();
        if (v->getType() == Binary || v->getType() == Integer)
            if (!isIntegral(x_int_[idx])) return false;
    }
    return true;
}

// Step 5: Perturbation step (randomly flip some integers)
void FeasibilityPump::perturbationStep() {
    logger_->msgStream(LogInfo) << "FP:perturb: enter perturbationStep(), x_int_.size()=" << x_int_.size() << "\n";
    UInt n = p_->getNumVars();
    UInt nFlip = std::max((UInt)1, n / 10);
    for (UInt i = 0; i < nFlip; ++i) {
        UInt idx = rand() % n;
        x_int_[idx] = 1 - x_int_[idx];  // flip binary
    }
}

// Step 6: Modify NLP objective to ||x - x_int||
void FeasibilityPump::updateObjectiveToDistance() {
    logger_->msgStream(LogInfo) << "FP:updateObj: enter updateObjectiveToDistance()\n";

    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    UInt i = 0;
    for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); ++it, ++i) {
        double coeff = 2.0 * (x_cont_[i] - x_int_[i]);
        lf->addTerm(*it, coeff);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    p_->changeObj(f, 0.0);
}
void FeasibilityPump::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  solve(s_pool);  // Just call your existing implementation
}


// Step 7: Solve function (the main function)
void FeasibilityPump::solve(SolutionPoolPtr s_pool) {
    logger_->msgStream(LogInfo) << "Starting modular Feasibility Pump..." << std::endl;
    timer_->start();

    if (!s_pool) {
        logger_->msgStream(LogError) << "Error: Solution pool is null!" << std::endl;
        return;
    }

    if (!p_) {
        logger_->msgStream(LogError) << "Error: Problem pointer p_ is null!" << std::endl;
        return;
    }

    logger_->msgStream(LogInfo) << "FP: p_=" << (void*)p_ << " e_=" << (void*)e_ << std::endl;

    if (!initialize()) {
        logger_->msgStream(LogInfo) << "Initialization failed." << std::endl;
        return;
    }

    UInt iter = 0;
    while (iter < maxIter_ && timer_->query() < maxTime_) {
        updateObjectiveToDistance();
        if (!projectionStep()) break;
        roundingStep();

        if (isFeasible()) {
            if (x_int_.empty()) {
                logger_->msgStream(LogError) << "Error: x_int_ is empty!" << std::endl;
                return;
            }

            int err = 0;
            double obj = p_->getObjValue(&x_int_[0], &err);
            if (err) {
                logger_->msgStream(LogError) << "Error evaluating objective." << std::endl;
                return;
            }

            ConstSolutionPtr sol = (ConstSolutionPtr) new Solution(obj, &x_int_[0], p_);
            s_pool->addSolution(sol);
            logger_->msgStream(LogInfo) << "Feasible integer solution found!" << std::endl;
            break;
        }

        if (iter % 5 == 0) perturbationStep();
        ++iter;
    }

    logger_->msgStream(LogInfo)
        << "Feasibility Pump finished after " << iter
        << " iterations, time = " << timer_->query() << "s" << std::endl;
}

void FeasibilityPump::writeStats(std::ostream &out) const {
    out << "Feasibility Pump stats not implemented yet." << std::endl;
}
