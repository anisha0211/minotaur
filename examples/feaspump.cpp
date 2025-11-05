#include "Environment.h"
#include "Problem.h"
#include "Variable.h"
#include "LinearFunction.h"
#include "Function.h"
#include "Objective.h"
#include "SolutionPool.h"
#include "OsiLPEngine.h"
#include "FeasibilityPump.h"
#include "Timer.h"
#include <iostream>

using namespace Minotaur;

int main()
{
  // --- Environment + timer ---
  EnvPtr env = (EnvPtr) new Environment();
  Timer *global_timer = env->getNewTimer();
  global_timer->start();

  // --- Create problem ---
  ProblemPtr p = (ProblemPtr) new Problem(env);

  // Variables: integer variables x1, x2 in [0,10]
  VariablePtr x1 = p->newVariable(0.0, 10.0, Integer, "x1");
  VariablePtr x2 = p->newVariable(0.0, 10.0, Integer, "x2");

  // Constraint: x1 + 2*x2 <= 14
  LinearFunctionPtr lf_con = (LinearFunctionPtr) new LinearFunction();
  lf_con->addTerm(x1, 1.0);
  lf_con->addTerm(x2, 2.0);
  FunctionPtr f_con = (FunctionPtr) new Function(lf_con);
  p->newConstraint(f_con, -INFINITY, 14.0, "c1");

  // Objective: minimize x1 + x2
  LinearFunctionPtr lf_obj = (LinearFunctionPtr) new LinearFunction();
  lf_obj->addTerm(x1, 1.0);
  lf_obj->addTerm(x2, 1.0);
  FunctionPtr f_obj = (FunctionPtr) new Function(lf_obj);
  p->newObjective(f_obj, 0.0, Minimize);  // ✅ correct usage

  // --- Create a concrete engine (OsiLPEngine) ---
  EnginePtr e = (EnginePtr) new OsiLPEngine(env);

  // --- Create Feasibility Pump instance ---
  FeasibilityPump fp(env, p, e);

  // --- Create solution pool ---
  SolutionPoolPtr s_pool = (SolutionPoolPtr) new SolutionPool(env, p, 10);

  // --- Run the Feasibility Pump ---
  fp.solve(s_pool);

  // --- Results ---
  UInt nsol = s_pool->getNumSols();  // ✅ correct method

  std::cout << "Feasibility Pump test completed." << std::endl;
  std::cout << "Number of solutions in pool: " << nsol << std::endl;

  // --- Stop timer ---
  global_timer->stop();
  std::cout << "Total time: " << global_timer->query() << " s" << std::endl;

  delete env;
  return 0;
}
