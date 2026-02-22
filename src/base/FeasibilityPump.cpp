// ============================================================================
// Enhanced Feasibility Pump for Convex MINLP
// ============================================================================

#include "MinotaurConfig.h"
#include "FeasibilityPump.h"
#include "Environment.h"
#include "Problem.h"
#include "Engine.h"
#include "Logger.h"
#include "Timer.h"
#include "SolutionPool.h"
#include "Relaxation.h"
#include "LinearFunction.h"
#include "Function.h"
#include "Objective.h"
#include "Constraint.h"
#include "Variable.h"
#include "Option.h"
#include <vector>
#include <cmath>
#include <set>

using namespace Minotaur;

// Constructor (e1: NLP engine, e2: MILP engine)

FeasibilityPump::FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr nlpEng,
                                 EnginePtr milpEng)
  : env_(env),
    p_(p),
    e1_(nlpEng),
    e2_(milpEng),
    intTol_(1e-6)
{
  logger_ = env_->getLogger();
  timer_ = env_->getNewTimer();
  n_ = p->getNumVars();

  // Collect integer vars
  for (VariableConstIterator it = p->varsBegin(); it != p->varsEnd(); ++it) {
    if ((*it)->getType() == Binary || (*it)->getType() == Integer) {
      intVars_.push_back(*it);
    }
  }

  // Ensure Jacobian exists ONCE
  if (!p_->getJacobian()) {
    p_->prepareForSolve();
  }
}
//random functions to ensure utility
bool FeasibilityPump::isFrac_(const double *x) const
{
  // Simple check: any integer variable fractional?
  for (size_t i = 0; i < intVars_.size(); ++i) {
    UInt idx = intVars_[i]->getIndex();
    if (fabs(x[idx] - round(x[idx])) > intTol_) {
      return true;
    }
  }
  return false;
}

bool FeasibilityPump::cycle_(UInt) const
{
  return false;
}

UInt FeasibilityPump::hash_() const
{
  return 0;
}

void FeasibilityPump::perturb_(UInt, UInt)
{
  // do nothing
}

void FeasibilityPump::convertSol_(SolutionPool*, const Solution*) const
{
  // do nothing
}

void FeasibilityPump::writeStats(std::ostream &) const
{
  // do nothing
}
// Utility Checks

bool FeasibilityPump::hasInteger_() const
{
  return !intVars_.empty();
}

bool FeasibilityPump::isMILP_() const
{
  ConstProblemSizePtr sz = p_->getSize();
  return (sz->nonlinCons == 0 && hasInteger_());
}

bool FeasibilityPump::isMINLP_() const
{
  ConstProblemSizePtr sz = p_->getSize();
  return (sz->nonlinCons > 0 && hasInteger_());
}

bool FeasibilityPump::isIntegerFeasible_(const double *x) const
{
  for (size_t i = 0; i < intVars_.size(); ++i) {
    UInt idx = intVars_[i]->getIndex();
    if (fabs(x[idx] - round(x[idx])) > intTol_)
      return false;
  }
  return true;
}

bool FeasibilityPump::isNonlinearFeasible_(const double *x) const
{
  for (ConstraintConstIterator it = p_->consBegin();
       it != p_->consEnd(); ++it) {
    int err = 0;
    double val = (*it)->getFunction()->eval(x, &err);
    if (err)
      return false;

    if (val < (*it)->getLb() - 1e-6)
      return false;
    if (val > (*it)->getUb() + 1e-6)
      return false;
  }
  return true;
}

// ============================================================================
// Solve function
// ============================================================================

void FeasibilityPump::solve(NodePtr, RelaxationPtr, SolutionPoolPtr sPool)
{
  if (!hasInteger_())
    return;

  timer_->start();

  if (isMILP_()) {
    solveMILP_(sPool);
    return;
  }

  if (isMINLP_()) {
    solveMINLP_(sPool);
  }
}

// Tackle MILP case first (solve directly using MILP engine)

void FeasibilityPump::solveMILP_(SolutionPoolPtr sPool)
{
  e2_->clear();
  e2_->load(p_);

  EngineStatus st = e2_->solve();

  if (st == ProvenOptimal || st == ProvenLocalOptimal) {
    sPool->addSolution(e2_->getSolution());
  }
}

// MINLP Case (Enhanced version)

void FeasibilityPump::solveMINLP_(SolutionPoolPtr sPool)
{
  // Continuous NLP (initial relaxation step)
  //OptionDBPtr options = env_->getOptions();
  // RelaxationPtr rel=(RelaxationPtr) new Relaxation(p_,env_);
  // rel->calculateSize();
  // if(options->findBool("use_native_cgraph")->getValue() || rel->isQP() ||
  //   rel->isQuadratic()) {
  //   rel->setNativeDer();
  // } else {
  //   rel->setJacobian(p_->getJacobian());
  //   rel->setHessian(p_->getHessian());
  // }
  e1_->clear();
  e1_->load(p_);

  EngineStatus st = e1_->solve();
  if (st != ProvenOptimal && st != ProvenLocalOptimal)
    return;

  ConstSolutionPtr sol = e1_->getSolution();
  std::vector<double> xk(sol->getPrimal(), sol->getPrimal() + n_);

  if (isIntegerFeasible_(xk.data()) && isNonlinearFeasible_(xk.data())) {
    sPool->addSolution(sol);
    return;
  }

  const UInt maxIter = 100;
  UInt iter = 0;

  while (iter < maxIter) {
    ++iter;
    
    // Build OA (Outer Approximation) MILP, (FP-OA)

    ProblemPtr oaProb = p_->clone(env_);
    oaProb->prepareForSolve();
    buildL1Objective_(oaProb, xk);
    addOACuts_(oaProb, xk);
    addSeparationCuts_(oaProb);  // enhance FP, convex feasible region overall (gi's independently might be non-convex)

    e2_->clear();
    e2_->load(oaProb);

    EngineStatus st2 = e2_->solve();
    if (st2 != ProvenOptimal && st2 != ProvenLocalOptimal)
      break;

    ConstSolutionPtr milpSol = e2_->getSolution();
    std::vector<double> xhat(milpSol->getPrimal(), milpSol->getPrimal() + n_);

    if (isNonlinearFeasible_(xhat.data())) {

      int err = 0;
      double obj = p_->getObjValue(xhat.data(), &err);

      if (!err) {
        sPool->addSolution(
            ConstSolutionPtr(new Solution(obj, xhat.data(), p_)));
      }

      break;
    }

    // L2 Projection, (FP-NLP)

    ProblemPtr projProb = p_->clone(env_);
    projProb->prepareForSolve();
    buildL2Objective_(projProb, xhat);

    e1_->clear();
    e1_->load(projProb);

    EngineStatus st3 = e1_->solve();
    if (st3 != ProvenOptimal && st3 != ProvenLocalOptimal)
      break;

    ConstSolutionPtr projSol = e1_->getSolution();
    xk.assign(projSol->getPrimal(), projSol->getPrimal() + n_);

    if (isIntegerFeasible_(xk.data()) && isNonlinearFeasible_(xk.data())) {
      sPool->addSolution(projSol);
      break;
    }

    storeSeparationCut_(xk, xhat);
  }
}

// L1 (manhattan distance, L1 norm) MILP Objective

void FeasibilityPump::buildL1Objective_(ProblemPtr p,
                                        const std::vector<double> &xk)
{
  LinearFunctionPtr obj(new LinearFunction());

  for (size_t i = 0; i < intVars_.size(); ++i) {
    UInt idx = intVars_[i]->getIndex();
    VariablePtr v = p->getVariable(idx);

    VariablePtr t = p->newVariable(0, INFINITY, Continuous);

    LinearFunctionPtr c1(new LinearFunction());
    c1->addTerm(v, 1);
    c1->addTerm(t, -1);
    p->newConstraint(FunctionPtr(new Function(c1)), -INFINITY, xk[idx]);

    LinearFunctionPtr c2(new LinearFunction());
    c2->addTerm(v, -1);
    c2->addTerm(t, -1);
    p->newConstraint(FunctionPtr(new Function(c2)), -INFINITY, -xk[idx]);

    obj->addTerm(t, 1);
  }

  p->changeObj(FunctionPtr(new Function(obj)), 0);
}

// OA Gradient Cuts (Linearization of constraints via taylor's polynomial)

void FeasibilityPump::addOACuts_(ProblemPtr oaProb,
                                 const std::vector<double> &xk)
{
  for (ConstraintConstIterator it = p_->consBegin();
       it != p_->consEnd(); ++it) {
    ConstraintPtr c = *it;

    if (c->getFunction()->getLinearFunction())
      continue;

    int error = 0;

    double gval = c->getFunction()->eval(xk.data(), &error);
    if (error)
      continue;

    std::vector<double> grad(n_, 0.0);

    c->getFunction()->evalGradient(xk.data(), grad.data(), &error);
    if (error)
      continue;

    LinearFunctionPtr lf(new LinearFunction());

    double rhs = c->getUb() - gval;

    for (UInt i = 0; i < n_; ++i) {
      if (fabs(grad[i]) > 1e-12) {
        VariablePtr v = oaProb->getVariable(i);
        lf->addTerm(v, grad[i]);
        rhs += grad[i] * xk[i];
      }
    }

    oaProb->newConstraint(FunctionPtr(new Function(lf)), -INFINITY, rhs);
  }
}

// L2 (L2 norm - 2nd degree convex objective) Projection Objective

void FeasibilityPump::buildL2Objective_(ProblemPtr p,
                                        const std::vector<double> &xhat)
{
  LinearFunctionPtr lf(new LinearFunction());
  double c = 0.0;

  for (size_t i = 0; i < intVars_.size(); ++i) {
    UInt idx = intVars_[i]->getIndex();
    VariablePtr v = p->getVariable(idx);

    double t = xhat[idx];
    lf->addTerm(v, -2.0 * t);
    c += t * t;
  }

  p->changeObj(FunctionPtr(new Function(lf)), c);
}

// Separation Cuts (to tackle the last straw)

void FeasibilityPump::storeSeparationCut_(const std::vector<double> &xk,
                                          const std::vector<double> &xhat)
{
  sepCuts_.push_back(std::make_pair(xk, xhat));
}

void FeasibilityPump::addSeparationCuts_(ProblemPtr p)
{
  for (size_t k = 0; k < sepCuts_.size(); ++k) {
    const std::vector<double> &xk = sepCuts_[k].first;
    const std::vector<double> &xhat = sepCuts_[k].second;

    LinearFunctionPtr lf(new LinearFunction());
    double rhs = 0.0;

    for (size_t i = 0; i < intVars_.size(); ++i) {
      UInt idx = intVars_[i]->getIndex();
      double coef = xk[idx] - xhat[idx];

      VariablePtr v = p->getVariable(idx);
      lf->addTerm(v, coef);

      rhs += coef * xk[idx];
    }

    p->newConstraint(FunctionPtr(new Function(lf)), rhs, INFINITY);
  }
}
