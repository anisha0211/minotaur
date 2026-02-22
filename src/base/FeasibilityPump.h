// ============================================================================
// Enhanced Feasibility Pump for Convex MINLP
// ============================================================================

#ifndef MINOTAUR_FEASIBILITY_PUMP_H
#define MINOTAUR_FEASIBILITY_PUMP_H

#include "Environment.h"
#include "Problem.h"
#include "Engine.h"
#include "Logger.h"
#include "Timer.h"
#include "SolutionPool.h"
#include "Relaxation.h"
#include "Heuristic.h"
#include <vector>
#include <utility>

namespace Minotaur{

 struct FeasPumpStats {
   UInt   numNLPs;
   UInt   errors;
   UInt   numCycles;
   double time;
   double bestObjValue;
 };



class FeasibilityPump: public Heuristic {
public:
  // Constructor
  FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr nlpEngine,
                  EnginePtr milpEngine);

  // Main entry
  void solve(NodePtr node, RelaxationPtr rel, SolutionPoolPtr sPool);
  virtual void writeStats(std::ostream &out) const ;

private:
  // Core solve branches
  void solveMILP_(SolutionPoolPtr sPool);
  void solveMINLP_(SolutionPoolPtr sPool);

  // Objective builders
  void buildL1Objective_(ProblemPtr p, const std::vector<double> &xk);

  void buildL2Objective_(ProblemPtr p, const std::vector<double> &xhat);

  // Outer Approximation cuts
  void addOACuts_(ProblemPtr p, const std::vector<double> &xk);

  // Enhanced separation cuts
  void storeSeparationCut_(const std::vector<double> &xk,
                           const std::vector<double> &xhat);

  void addSeparationCuts_(ProblemPtr p);

  // Feasibility checks
  bool isIntegerFeasible_(const double *x) const;
  bool isNonlinearFeasible_(const double *x) const;

  bool hasInteger_() const;
  bool isMILP_() const;
  bool isMINLP_() const;

protected:
  // Environment
  EnvPtr env_;
  ProblemPtr p_;
  EnginePtr e1_; //NLP Engine
  EnginePtr e2_; //MILP Engine

  LoggerPtr logger_;
  Timer *timer_;
  

  // ----- Helper functions required by LinFeasPump -----------------------
  bool   isFrac_(const double* x) const;   // true if any integer variable is fractional
  void   convertSol_(SolutionPoolPtr s_pool, ConstSolutionPtr sol) const;
  UInt   hash_() const;                    // simple hash of roundedSol_
  bool   cycle_(UInt h) const;             // detect cycling (very small map)
  std::vector<double> roundedSol_;
  void   perturb_(UInt h, UInt nflip);    // flip nflip variables
  // Problem structure
  std::vector<VariablePtr> intVars_;
  UInt n_;  // number of variables
  double intTol_;
  FeasPumpStats*      stats_;

  // Separation cuts storage
  std::vector<std::pair<std::vector<double>, std::vector<double>>> sepCuts_;


};
typedef FeasibilityPump* FeasibilityPumpPtr;
}
#endif
