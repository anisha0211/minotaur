//
// Minotaur – Feasibility Pump (MILP + MINLP)
// Header file
//
#ifndef MINOTAUR_FEASIBILITYPUMP_H
#define MINOTAUR_FEASIBILITYPUMP_H
#include "Heuristic.h"
#include "Handler.h"
#include "Problem.h"
#include "SolutionPool.h"
#include "Types.h"
#include <memory>          // std::shared_ptr
#include <vector>

namespace Minotaur {

class Engine;
class Environment;
class Node;
class Relaxation;
class Timer;

// ---------------------------------------------------------------------------
// Statistics
// ---------------------------------------------------------------------------
struct FeasPumpStats {
  UInt   numNLPs;
  UInt   errors;
  UInt   numCycles;
  double time;
  double bestObjValue;
};

// ---------------------------------------------------------------------------
// FeasibilityPump
// ---------------------------------------------------------------------------
class FeasibilityPump : public Heuristic {
public:
  // MILP constructor – the one Minotaur calls
  FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e2);

  // MINLP constructor – pass both engines
  FeasibilityPump(EnvPtr env, ProblemPtr p,
                  EnginePtr e1, EnginePtr e2);

  virtual ~FeasibilityPump();

  // ----- Handler interface -------------------------------------------------
  // NOTE: **no parameter names** – must match the pure-virtual base
  virtual void solve(NodePtr, RelaxationPtr, SolutionPoolPtr) override;
  virtual void writeStats(std::ostream &out) const override;

  // ----- Helper ------------------------------------------------------------
  bool shouldFP_() const;
protected:                     // <--- everything below is protected
  // ----- Engines -----------------------------------------------------------
  EnginePtr e2_;   // LP/MILP engine
  EnginePtr e1_;  // NLP engine (MINLP only)

  // ----- (rest of the members you already have) -------------------------
  EnvPtr    env_;
  ProblemPtr p_;
  LoggerPtr logger_;
  Timer*    timer_;
  double    intTol_;
  UInt      nToFlip_;
  VarVector intVars_;
  std::vector<double> contSol_;
  std::vector<double> roundedSol_;
  std::vector<double> projSol_;
  FeasPumpStats*      stats_;
  std::vector<std::vector<double>> oaPoints_;
  std::vector<ConstraintPtr>       oaCuts_;
  std::vector<double>              oaRhs_;

  // ----- Helper functions required by LinFeasPump -----------------------
  bool   isFrac_(const double* x) const;   // true if any integer variable is fractional
  void   convertSol_(SolutionPoolPtr s_pool, ConstSolutionPtr sol) const;
  UInt   hash_() const;                    // simple hash of roundedSol_
  bool   cycle_(UInt h) const;             // detect cycling (very small map)
  void   perturb_(UInt h, UInt nflip);    // flip nflip variables

  // ----- (your private helpers) -----------------------------------------
  void initCommon_();
  bool solveRelaxation_(EnginePtr engine, const double*& x);
  void randomRound_(const double* contX);
  bool l1Projection_();
  bool oaRounding_();
  bool l2Projection_();
  bool isFeasible_() const;

  static const std::string me_;
};

// ---------------------------------------------------------------------------
// Smart pointer – std::shared_ptr (no boost)
// ---------------------------------------------------------------------------
typedef std::shared_ptr<FeasibilityPump> FeasibilityPumpPtr;

} // namespace Minotaur

#endif // MINOTAUR_FEASIBILITYPUMP_H
