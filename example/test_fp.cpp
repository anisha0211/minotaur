#include "MinotaurConfig.h"
#include "FeasibilityPump.h"
#include "Problem.h"
#include "SolutionPool.h"
#include "Environment.h"
#include "IpoptEngine.h"
#include "AMPLInterface.h"
#include <iostream>

using namespace Minotaur;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: test_fp problem.nl\n";
        return 1;
    }

    // Initialize environment
    EnvPtr env = std::make_shared<Environment>();

    // Load problem using AMPL interface
    AMPLInterface* ampl = new AMPLInterface(env);
    ProblemPtr problem = ampl->readNL(argv[1]);

    // Create an NLP engine for solving relaxed problems
    EnginePtr engine = std::make_shared<IpoptEngine>(env);

    // Create a solution pool
    SolutionPoolPtr s_pool = std::make_shared<SolutionPool>();

    // Create Feasibility Pump object
    FeasibilityPump fp(env, problem, engine);

    // Solve using FP only
    fp.solve(nullptr, nullptr, s_pool);

    // Print all solutions found
    s_pool->write(std::cout);

    return 0;
}
