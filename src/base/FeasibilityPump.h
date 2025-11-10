#ifndef MINOTAUR_FEASIBILITYPUMP_H
#define MINOTAUR_FEASIBILITYPUMP_H

#include "Environment.h"
#include "Engine.h"
#include "Problem.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"
#include "Logger.h"
#include "Heuristic.h"

#include <vector>
#include <cmath>
#include <memory>
#include <iostream>

namespace Minotaur {

class FeasibilityPump : public Heuristic {
public:
  // Constructor / Destructor
  FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e);
  ~FeasibilityPump() override = default;
  FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e1, EnginePtr e2)
  : env_(env), p_(p), e_(e1) {
    (void)e2;  // unused for base
  }

  // --- Heuristic Interface ---
  void solve(NodePtr node, RelaxationPtr rel, SolutionPoolPtr s_pool) override;

  // Optional helper version for standalone testing
  virtual void solve(SolutionPoolPtr s_pool);
  virtual void writeStats(std::ostream &out) const override;

protected:
  // --- Core Members ---
  EnvPtr env_;
  ProblemPtr p_;
  EnginePtr e_;
  LoggerPtr logger_;
  Timer* timer_;

  double intTol_;
  UInt maxIter_;
  double maxTime_;
  std::vector<double> x_cont_;
  std::vector<double> x_int_;

  // --- Core Steps ---
  virtual bool initialize();
  virtual bool projectionStep();
  virtual void roundingStep();
  virtual bool isFeasible() const;
  virtual void perturbationStep();
  virtual void updateObjectiveToDistance();

  bool isIntegral(double x) const { return fabs(x - round(x)) <= intTol_; }

  // --- Empty Virtual Stubs for LinFeasPump ---
  // These do nothing in base class, but are here so LinFeasPump can override them.
  virtual void constructObj_(ProblemPtr, ConstSolutionPtr) {}
  virtual void implementFP_(const double*, SolutionPoolPtr) {}
  virtual double getSolGap_(double, double) { return 0.0; }
  virtual bool prepareLP_(SolutionPool*) { return false; }
  virtual void separatingCut_(double, SolutionPoolPtr) {}
  virtual bool shouldFP_() { return false; }
  std::vector<double> roundedSol_;  // used by LinFeasPump
  // --- Add this in FeasibilityPump.h ---
  struct FeasStats {
      UInt numNLPs = 0;      // already used
      UInt numCycles = 0;    // 👈 new field expected by LinFeasPump
      double time = 0.0;
  };

  FeasStats* stats_ = nullptr;

  // Methods LinFeasPump expects
  virtual bool isFrac_(const double* /*x*/) { return false; }
  virtual void convertSol_(SolutionPoolPtr, ConstSolutionPtr) {}
  virtual size_t hash_() { return 0; }
  virtual bool cycle_(size_t) { return false; }
  virtual void perturb_(size_t, UInt) {}

  };
  typedef FeasibilityPump* FeasibilityPumpPtr;

} // namespace Minotaur

#endif // MINOTAUR_FEASIBILITYPUMP_H
