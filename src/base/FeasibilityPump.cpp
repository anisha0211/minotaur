//
// Minotaur – Feasibility Pump (MILP + MINLP)
//
#include "MinotaurConfig.h"
#include "Constraint.h"
#include "Engine.h"
#include "Environment.h"
#include "FeasibilityPump.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Node.h"
#include "SolutionPool.h"
#include "Objective.h"
#include "ProblemSize.h"
#include "Timer.h"
#include "Types.h"
#include "Variable.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <vector>

using namespace Minotaur;

// ---------------------------------------------------------------------------
// static
// ---------------------------------------------------------------------------
const std::string FeasibilityPump::me_ = "FeasibilityPump: ";

// ---------------------------------------------------------------------------
// constructors
// ---------------------------------------------------------------------------
FeasibilityPump::FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e2)
  : e2_(e2), e1_(EnginePtr()), env_(env),
    intTol_(1e-6), nToFlip_(0), p_(p), stats_(0)
{
  stats_ = new FeasPumpStats();      // ← ADD THIS LINE
  stats_->numNLPs = stats_->errors = stats_->numCycles = 0;
  stats_->time = 0.0;
  stats_->bestObjValue = INFINITY;
  initCommon_();
}

FeasibilityPump::FeasibilityPump(EnvPtr env, ProblemPtr p,
                                 EnginePtr e1, EnginePtr e2)
  : e2_(e2), e1_(e1), env_(env),
    intTol_(1e-6), nToFlip_(0), p_(p), stats_(0)
{
  stats_ = new FeasPumpStats();      // ← ADD THIS LINE
  stats_->numNLPs = stats_->errors = stats_->numCycles = 0;
  stats_->time = 0.0;
  stats_->bestObjValue = INFINITY;
  initCommon_();
}


void FeasibilityPump::initCommon_()
{
  srand(1);
  logger_ = env_->getLogger();
  timer_  = env_->getNewTimer();

  intVars_.clear();
  for (VariableConstIterator v = p_->varsBegin(); v != p_->varsEnd(); ++v) {
    if ((*v)->getType() == Binary || (*v)->getType() == Integer) {
      intVars_.push_back(*v);
    }
  }

  contSol_.resize(p_->getNumVars(), 0.0);
  roundedSol_.resize(p_->getNumVars(), 0.0);
  projSol_.resize(p_->getNumVars(), 0.0);

  stats_ = new FeasPumpStats();
  stats_->numNLPs = 0;
  stats_->errors  = 0;
  stats_->numCycles = 0;
  stats_->time    = 0.0;
  stats_->bestObjValue = INFINITY;
}

// ---------------------------------------------------------------------------
// destructor
// ---------------------------------------------------------------------------
FeasibilityPump::~FeasibilityPump()
{
  delete stats_;
  if (timer_) delete timer_;
}

//random functions
bool FeasibilityPump::isFrac_(const double* x) const
{
  for (size_t i = 0; i < intVars_.size(); ++i) {
    double val = x[intVars_[i]->getIndex()];
    double frac = val - std::floor(val);
    if (frac > intTol_ && frac < 1.0 - intTol_) return true;
  }
  return false;
}

void FeasibilityPump::convertSol_(SolutionPoolPtr s_pool,
                                  ConstSolutionPtr sol) const
{
  int err = 0;
  double obj = p_->getObjValue(sol->getPrimal(), &err);
  if (!err) {
    s_pool->addSolution(ConstSolutionPtr(
          new Solution(obj, sol->getPrimal(), p_)));
  }
}

UInt FeasibilityPump::hash_() const
{
  UInt h = 0;
  for (size_t i = 0; i < intVars_.size(); ++i) {
    double v = roundedSol_[intVars_[i]->getIndex()];
    h = h * 31 + static_cast<UInt>(std::round(v));
  }
  return h;
}

bool FeasibilityPump::cycle_(UInt h) const
{
  static std::set<UInt> seen;
  if (seen.count(h)) return true;
  seen.insert(h);
  if (seen.size() > 100) seen.clear();   // avoid unbounded growth
  return false;
}

void FeasibilityPump::perturb_(UInt, UInt nflip)
{
  std::vector<size_t> idx(intVars_.size());
  for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
  std::random_shuffle(idx.begin(), idx.end());

  for (UInt i = 0; i < std::min(nflip, (UInt)idx.size()); ++i) {
    VariablePtr v = intVars_[idx[i]];
    double val = roundedSol_[v->getIndex()];
    roundedSol_[v->getIndex()] = (std::abs(val - std::floor(val)) < intTol_) ?
                                 std::ceil(val) : std::floor(val);
  }
}


// ---------------------------------------------------------------------------
// Step 1 – solve relaxation (LP or NLP)
// ---------------------------------------------------------------------------
bool FeasibilityPump::solveRelaxation_(EnginePtr engine, const double*& x)
{
  engine->clear();
  engine->load(p_);
  EngineStatus st = engine->solve();
  ++stats_->numNLPs;

  if (st != ProvenOptimal && st != ProvenLocalOptimal) {
    logger_->msgStream(LogInfo) << me_ << "Relaxation failed (status=" << st << ")\n";
    return false;
  }

  ConstSolutionPtr sol = engine->getSolution();
  x = sol->getPrimal();
  return true;
}

// ---------------------------------------------------------------------------
// Step 2 – random rounding (MILP only)
// ---------------------------------------------------------------------------
void FeasibilityPump::randomRound_(const double* contX)
{
  std::fill(roundedSol_.begin(), roundedSol_.end(), 0.0);

  UInt idx = 0;
  for (VariableConstIterator v = p_->varsBegin(); v != p_->varsEnd(); ++v, ++idx) {
    if ((*v)->getType() == Binary || (*v)->getType() == Integer) {
      double val = contX[idx];
      double floorV = std::floor(val);
      double ceilV  = std::ceil(val);

      if (std::abs(val - floorV) < intTol_) { roundedSol_[idx] = floorV; continue; }
      if (std::abs(val - ceilV)  < intTol_) { roundedSol_[idx] = ceilV;  continue; }

      bool roundUp = (rand() % 2 == 0);
      roundedSol_[idx] = roundUp ? ceilV : floorV;
    } else {
      roundedSol_[idx] = contX[idx];
    }
  }
}

// ---------------------------------------------------------------------------
// Step 3 – feasibility check
// ---------------------------------------------------------------------------
bool FeasibilityPump::isFeasible_() const
{
  int err = 0;
  p_->getObjValue(roundedSol_.data(), &err);
  if (err) return false;

  for (ConstraintConstIterator it = p_->consBegin(); it != p_->consEnd(); ++it) {
    ConstraintPtr c = *it;
    double lb = c->getLb();
    double ub = c->getUb();

    int error = 0;
    double val = c->getFunction()->eval(roundedSol_.data(), &error);
    if (error) {
      logger_->msgStream(LogDebug) << me_ << "Constraint eval error (non-linear?).\n";
      return false;
    }

    if ((lb > -INFINITY && val < lb - intTol_) ||
        (ub <  INFINITY && val > ub + intTol_)) {
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
// Step 4 – L1 projection (MILP)
// ---------------------------------------------------------------------------
bool FeasibilityPump::l1Projection_()
{
  std::vector<VariablePtr> auxVars;
  std::vector<ConstraintPtr> auxCons;
  LinearFunctionPtr objLF(new LinearFunction());

  for (size_t i = 0; i < intVars_.size(); ++i) {
    VariablePtr v = intVars_[i];
    UInt idx = v->getIndex();
    double target = roundedSol_[idx];

    VariablePtr t = p_->newVariable(0.0, INFINITY, Continuous);
    auxVars.push_back(t);

    // t >= x - target
    LinearFunctionPtr lf1(new LinearFunction());
    lf1->addTerm(v, 1.0);
    lf1->addTerm(t, -1.0);
    FunctionPtr f1(new Function(lf1));
    ConstraintPtr c1 = p_->newConstraint(f1, -INFINITY, target);
    auxCons.push_back(c1);

    // t >= -(x - target)
    LinearFunctionPtr lf2(new LinearFunction());
    lf2->addTerm(v, -1.0);
    lf2->addTerm(t, -1.0);
    FunctionPtr f2(new Function(lf2));
    ConstraintPtr c2 = p_->newConstraint(f2, -INFINITY, -target);
    auxCons.push_back(c2);

    objLF->addTerm(t, 1.0);
  }

  FunctionPtr objF(new Function(objLF));
  p_->changeObj(objF, 0.0);

  
  e2_->clear();
  e2_->load(p_);
  EngineStatus st = e2_->solve();
  ++stats_->numNLPs;

  // clean up
  for (VariablePtr t : auxVars) p_->markDelete(t);
  for (ConstraintPtr c : auxCons) p_->markDelete(c);
  p_->delMarkedCons();
  p_->delMarkedVars();
  p_->prepareForSolve();

  if (st != ProvenOptimal && st != ProvenLocalOptimal) {
    logger_->msgStream(LogInfo) << me_ << "L1 projection failed (status=" << st << ")\n";
    return false;
  }

  ConstSolutionPtr sol = e2_->getSolution();
  const double* pr = sol->getPrimal();
  std::copy(pr, pr + p_->getNumVars(), projSol_.begin());
  return true;
}

// ---------------------------------------------------------------------------
// MINLP – OA rounding
// ---------------------------------------------------------------------------
bool FeasibilityPump::oaRounding_()
{
  ProblemPtr oaProb = p_->clone(env_);
  LinearFunctionPtr objLF(new LinearFunction());

  // L1 slack variables
  std::vector<VariablePtr> slacks;
  for (size_t i = 0; i < intVars_.size(); ++i) {
    VariablePtr v = oaProb->getVariable(intVars_[i]->getIndex());
    double target = roundedSol_[intVars_[i]->getIndex()];

    VariablePtr t = oaProb->newVariable(0.0, INFINITY, Continuous);
    slacks.push_back(t);
    objLF->addTerm(t, 1.0);

    LinearFunctionPtr lf1(new LinearFunction());
    lf1->addTerm(v, 1.0);
    lf1->addTerm(t, -1.0);
    oaProb->newConstraint(FunctionPtr(new Function(lf1)), -INFINITY, target);

    LinearFunctionPtr lf2(new LinearFunction());
    lf2->addTerm(v, -1.0);
    lf2->addTerm(t, -1.0);
    oaProb->newConstraint(FunctionPtr(new Function(lf2)), -INFINITY, -target);
  }

  // Add previous linearizations
  for (size_t k = 0; k < oaCuts_.size(); ++k) {
    LinearFunctionPtr lf = oaCuts_[k]->getFunction()->getLinearFunction();
    if (lf) {
      oaProb->newConstraint(FunctionPtr(new Function(lf)), -INFINITY, oaRhs_[k]);
    }
  }

  oaProb->changeObj(FunctionPtr(new Function(objLF)), 0.0);

  e2_->clear();
  e2_->load(oaProb);
  EngineStatus st = e2_->solve();
  ++stats_->numNLPs;

  if (st != ProvenOptimal && st != ProvenLocalOptimal) {
    return false;
  }

  ConstSolutionPtr sol = e2_->getSolution();
  const double* x = sol->getPrimal();
  std::copy(x, x + p_->getNumVars(), roundedSol_.begin());

  oaPoints_.push_back(std::vector<double>(x, x + p_->getNumVars()));
  return true;
}

// ---------------------------------------------------------------------------
// MINLP – L2 projection
// ---------------------------------------------------------------------------
bool FeasibilityPump::l2Projection_()
{
  ProblemPtr projProb = p_->clone(env_);

  LinearFunctionPtr objLF(new LinearFunction());
  double constPart = 0.0;

  for (size_t i = 0; i < intVars_.size(); ++i) {
    VariablePtr v = projProb->getVariable(intVars_[i]->getIndex());
    double target = roundedSol_[intVars_[i]->getIndex()];
    objLF->addTerm(v, -2.0 * target);
    constPart += target * target;
  }

  projProb->changeObj(FunctionPtr(new Function(objLF)), constPart);

  e1_->clear();
  e1_->load(projProb);
  EngineStatus st = e1_->solve();
  ++stats_->numNLPs;

  if (st != ProvenOptimal && st != ProvenLocalOptimal) {
    return false;
  }

  ConstSolutionPtr sol = e1_->getSolution();
  const double* pr = sol->getPrimal();
  std::copy(pr, pr + p_->getNumVars(), projSol_.begin());
  return true;
}

// ---------------------------------------------------------------------------
// shouldFP_ – detect MILP vs MINLP
// ---------------------------------------------------------------------------
bool FeasibilityPump::shouldFP_() const
{
  ConstProblemSizePtr sz = p_->getSize();
  bool isMILP  = (sz->nonlinCons == 0 && sz->ints > 0);
  bool isMINLP = (sz->nonlinCons > 0 && sz->ints > 0);
  return isMILP || isMINLP;
}

// ---------------------------------------------------------------------------
// main solve()
// ---------------------------------------------------------------------------
void FeasibilityPump::solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool)
{
  if (!shouldFP_()) {
    logger_->msgStream(LogInfo) << me_ << "Skipping (not MILP/MINLP)\n";
    return;
  }

  timer_->start();

  ConstProblemSizePtr sz = p_->getSize();
  bool isMILP = (sz->nonlinCons == 0 && sz->ints > 0);

  const double* x = nullptr;
  EnginePtr relaxEngine = isMILP ? e2_ : e1_;
  if (!solveRelaxation_(relaxEngine, x)) {
    stats_->time = timer_->query();
    return;
  }

  std::copy(x, x + p_->getNumVars(), contSol_.begin());

  // -----------------------------------------------------------------------
  // MILP path
  // -----------------------------------------------------------------------
  if (isMILP) {
    randomRound_(contSol_.data());

    if (isFeasible_()) {
      int err = 0;
      double obj = p_->getObjValue(roundedSol_.data(), &err);
      s_pool->addSolution(ConstSolutionPtr(new Solution(obj, roundedSol_.data(), p_)));
      logger_->msgStream(LogInfo) << me_ << "MILP: feasible after rounding.\n";
      stats_->time = timer_->query();
      return;
    }

    const UInt maxIter = 200;
    UInt iter = 0;
    while (iter < maxIter && timer_->query() < 60.0) {
      ++iter;
      randomRound_(x);
      if (isFeasible_()) {
        int err = 0;
        double obj = p_->getObjValue(roundedSol_.data(), &err);
        s_pool->addSolution(ConstSolutionPtr(new Solution(obj, roundedSol_.data(), p_)));
        logger_->msgStream(LogInfo) << me_ << "MILP: feasible solution (iter " << iter << ")\n";
        break;
      }
      if (!l1Projection_()) break;
      x = projSol_.data();
    }
  }
  // -----------------------------------------------------------------------
  // MINLP path
  // -----------------------------------------------------------------------
  else {
    oaPoints_.clear();
    oaCuts_.clear();
    oaRhs_.clear();

    std::copy(x, x + p_->getNumVars(), roundedSol_.begin());
    oaPoints_.push_back(std::vector<double>(x, x + p_->getNumVars()));

    const UInt maxIter = 50;
    UInt iter = 0;
    while (iter < maxIter && timer_->query() < 120.0) {
      ++iter;

      if (!oaRounding_()) {
        logger_->msgStream(LogInfo) << me_ << "MINLP: OA rounding failed.\n";
        break;
      }

      if (isFeasible_()) {
        int err = 0;
        double obj = p_->getObjValue(roundedSol_.data(), &err);
        s_pool->addSolution(ConstSolutionPtr(new Solution(obj, roundedSol_.data(), p_)));
        logger_->msgStream(LogInfo) << me_ << "MINLP: feasible solution (iter " << iter << ")\n";
        break;
      }

      if (!l2Projection_()) {
        logger_->msgStream(LogInfo) << me_ << "MINLP: L2 projection failed.\n";
        break;
      }

      std::copy(projSol_.begin(), projSol_.end(), roundedSol_.begin());
      oaPoints_.push_back(std::vector<double>(projSol_.begin(), projSol_.end()));
    }
  }

  stats_->time = timer_->query();
}

// ---------------------------------------------------------------------------
// statistics
// ---------------------------------------------------------------------------
void FeasibilityPump::writeStats(std::ostream &out) const
{
  out << me_ << "NLPs solved = " << stats_->numNLPs << "\n"
      << me_ << "time (s)   = " << stats_->time << "\n"
      << me_ << "OA cuts    = " << oaCuts_.size() << "\n";
}
