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
  OptionDBPtr options = env_->getOptions();
  RelaxationPtr rel=(RelaxationPtr) new Relaxation(p_,env_);
  rel->calculateSize();
  if(options->findBool("use_native_cgraph")->getValue() || rel->isQP() ||
     rel->isQuadratic()) {
     rel->setNativeDer();
  } else {
     rel->setJacobian(p_->getJacobian());
     rel->setHessian(p_->getHessian());
  }
  e1_->clear();
  e1_->load(rel);

  EngineStatus st = e1_->solve();
  if (st != ProvenOptimal && st != ProvenLocalOptimal)
    return;
   
  ConstSolutionPtr sol = e1_->getSolution();
  if (sol && sol->getPrimal()) {
    sPool->addSolution(sol);   // ADD THIS
}  

  std::vector<double> xk(sol->getPrimal(), sol->getPrimal() + n_);

  if (isIntegerFeasible_(xk.data()) && isNonlinearFeasible_(xk.data())) {
    sPool->addSolution(sol);
    return;
  }



    
    //Checking solution at NLP Stage
    std::cout << "NLP xk:\n";
    for (UInt i=0; i<n_; ++i){
        std::cout << xk[i] << " ";}
       std::cout << std::endl;

    if (!sol || !sol->getPrimal()) {
    std::cout << "NLP returned NULL solution!\n";
    return;
    }
 


  const UInt maxIter = 100;
  UInt iter = 0;

  while (iter < maxIter) {
    ++iter;
    std::cout << "\n========== FP Iteration " << iter << " ==========\n";
    // Build OA (Outer Approximation) MILP, (FP-OA)

    ProblemPtr oaProb = p_->clone(env_);
    oaProb->prepareForSolve();
    buildL1Objective_(oaProb, xk);
    addOACuts_(oaProb, xk);
    addSeparationCuts_(oaProb);  // enhance FP, convex feasible region overall (gi's independently might be non-convex)

       // Create a linear-only problem (copy variables, linear objective, linear
    // constraints)
    {
      ProblemPtr linProb(new Problem(env_));

      // Build mapping from old variable indices -> new VariablePtr
      size_t maxOldIdx = 0;
      for (VariableConstIterator vit = oaProb->varsBegin();
           vit != oaProb->varsEnd(); ++vit) {
        VariablePtr v = *vit;
        if ((size_t)v->getIndex() > maxOldIdx) {
          maxOldIdx = v->getIndex();
        }
      }
      std::vector<VariablePtr> mapOldToNew(maxOldIdx + 1, VariablePtr());

      // copy variables (preserve indices / ordering by mapping)
      for (VariableConstIterator vit = oaProb->varsBegin();
           vit != oaProb->varsEnd(); ++vit) {
        VariablePtr v = *vit;
        VariablePtr v_new =
            linProb->newVariable(v->getLb(), v->getUb(), v->getType());
        mapOldToNew[v->getIndex()] = v_new;
      }

      // prepare a zero point for evaluations/gradients sized to OA problem
      UInt oaVarCnt = (UInt)(maxOldIdx + 1);
      std::vector<double> x0(oaVarCnt, 0.0);
      std::vector<double> grad(oaVarCnt, 0.0);

      // copy linear objective if present (use gradient + constant via eval)
      FunctionPtr oaf = oaProb->getObjective()->getFunction();
      if (oaf && oaf->getLinearFunction()) {
        int err = 0;
        std::fill(grad.begin(), grad.end(), 0.0);
        oaf->evalGradient(x0.data(), grad.data(), &err);
        if (!err) {
          LinearFunctionPtr nlf(new LinearFunction());
          // use mapping to add terms
          for (UInt i = 0; i < grad.size(); ++i) {
            if (fabs(grad[i]) > 1e-12 && i < mapOldToNew.size() &&
                mapOldToNew[i]) {
              nlf->addTerm(mapOldToNew[i], grad[i]);
            }
          }
          double val = 0.0;
          err = 0;
          val = oaf->eval(x0.data(), &err);
          if (!err) {
            linProb->changeObj(FunctionPtr(new Function(nlf)), val);
          } else {
            linProb->changeObj(FunctionPtr(new Function(nlf)), 0.0);
          }
        }
      }

      // copy only linear constraints (includes OA + separation cuts)
      for (ConstraintConstIterator cit = oaProb->consBegin();
           cit != oaProb->consEnd(); ++cit) {
        ConstraintPtr c = *cit;
        FunctionPtr cf = c->getFunction();
        if (!cf || !cf->getLinearFunction())
          continue;

        int err = 0;
        std::fill(grad.begin(), grad.end(), 0.0);
        cf->evalGradient(x0.data(), grad.data(), &err);
        if (err)
          continue;

        LinearFunctionPtr nclf(new LinearFunction());
        for (UInt i = 0; i < grad.size(); ++i) {
          if (fabs(grad[i]) > 1e-12 && i < mapOldToNew.size() &&
              mapOldToNew[i]) {
            nclf->addTerm(mapOldToNew[i], grad[i]);
          }
        }

        // compute constant term b = f(0)
        double b = 0.0;
        err = 0;
        b = cf->eval(x0.data(), &err);
        if (err)
          b = 0.0;

        double lb = c->getLb();
        double ub = c->getUb();
        double new_lb = (lb == -INFINITY) ? -INFINITY : lb - b;
        double new_ub = (ub == INFINITY) ? INFINITY : ub - b;

        linProb->newConstraint(FunctionPtr(new Function(nclf)), new_lb,
                               new_ub);
      }

      // use linear-only problem for MILP engine
      oaProb = linProb;
    }





 
    //For checking if oaProb has any non linear constraints
    for (ConstraintConstIterator c_iter = oaProb->consBegin();
     c_iter != oaProb->consEnd(); ++c_iter) {

    ConstraintPtr c = *c_iter;

    if (c->getFunctionType() != Linear) {
        std::cout << "Nonlinear constraint found: "
                  << c->getName()
                  << " type = " << c->getFunctionType()
                  << std::endl;
          }
    }    

    //continue
    e2_->load(oaProb);

    EngineStatus st2 = e2_->solve();
    //Checking MILP Status
    std::cout << "MILP status = " << st2 << std::endl;
    if (st2 != ProvenOptimal && st2 != ProvenLocalOptimal)
      break;

    ConstSolutionPtr milpSol = e2_->getSolution();


    //Adding solution to sPool
    if (milpSol && milpSol->getPrimal()){
        sPool->addSolution(milpSol);
    }


   
    //Checking if solution exists
    if (!milpSol || !milpSol->getPrimal()) {
    std::cout << "milpSol is NULL!" << std::endl;
    break;
}

const double* primal = milpSol->getPrimal();

if (!primal) {
    std::cout << "No primal solution available!" << std::endl;
    return;
}






    std::vector<double> xhat(milpSol->getPrimal(), milpSol->getPrimal() + n_);

     //Adding checks after MILP Solving
     if (!milpSol || !milpSol->getPrimal()) {
         std::cout << "MILP returned NULL solution!\n";
         continue;
     }

     std::cout << "\n=== MILP Solution xhat ===\n";
      for (UInt i = 0; i < n_; ++i) {
          std::cout << xhat[i] << " ";
     }
     std::cout << "\n=========================\n";



    if (isNonlinearFeasible_(xhat.data())) {

      int err = 0;
      double obj = p_->getObjValue(xhat.data(), &err);
      //Checking objective value
      std::cout << "Objective value at xhat = " << obj << std::endl;

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

    projProb->calculateSize();
    if(options->findBool("use_native_cgraph")->getValue() || projProb->isQP() ||
       projProb->isQuadratic()) {
       projProb->setNativeDer();
    } else {
       projProb->setJacobian(p_->getJacobian());
       projProb->setHessian(p_->getHessian());
    }



    e1_->clear();
    e1_->load(projProb);

    EngineStatus st3 = e1_->solve();
    if (st3 != ProvenOptimal && st3 != ProvenLocalOptimal)
      break;

    ConstSolutionPtr projSol = e1_->getSolution();

    if (projSol && projSol->getPrimal()){
       sPool->addSolution(sol);
    }
   
    xk.assign(projSol->getPrimal(), projSol->getPrimal() + n_);

    //Checking NLP Projection Result
    std::cout << "\n=== Projection xk ===\n";
    for (UInt i = 0; i < n_; ++i) {
        std::cout << xk[i] << " ";
    }
    std::cout << "\n====================\n";



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


    //Checking solution after OA Cuts
    std::cout << "OA cut: ";
     for (UInt i = 0; i < n_; ++i) {
        if (fabs(grad[i]) > 1e-6) {
            std::cout << grad[i] << "*x" << i << " ";
         }
     }
     std::cout << "<= " << rhs << std::endl;    
     
    if (error) {
         std::cout << "Gradient error in constraint, skipping\n";
         continue;
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
