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

namespace Minotaur {

class FeasibilityPump : public Heuristic
{
public:
  // Constructor / Destructor
  FeasibilityPump(EnvPtr env, ProblemPtr p, EnginePtr e);
  ~FeasibilityPump() override = default;

  // ✅ Override the same signature as Heuristic base
  void solve(NodePtr node, RelaxationPtr rel, SolutionPoolPtr s_pool) override;

  // Optional helper version for standalone testing
  void solve(SolutionPoolPtr s_pool);
  void writeStats(std::ostream &out) const override;

private:
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

  bool initialize();
  bool projectionStep();
  void roundingStep();
  bool isFeasible() const;
  void perturbationStep();
  void updateObjectiveToDistance();

  bool isIntegral(double x) const { return fabs(x - round(x)) <= intTol_; }
};

} // namespace Minotaur

#endif // MINOTAUR_FEASIBILITYPUMP_H
