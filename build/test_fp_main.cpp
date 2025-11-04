#include "minotaur/Environment.h"
#include "minotaur/Problem.h"
#include "minotaur/Engine.h"
#include "minotaur/IpoptEngine.h"
#include "minotaur/FeasibilityPump.h"
#include "minotaur/SolutionPool.h"
#include "minotaur/AMPLInterface.h"
#include "minotaur/Relaxation.h"
#include <iostream>
#include <csignal>

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "Segmentation fault occurred!" << std::endl;
    exit(signum);
}

int main(int argc, char** argv) {
    signal(SIGSEGV, signalHandler);
    
    if (argc < 2) {
        std::cout << "Usage: ./test_fp_main problem.nl" << std::endl;
        return 1;
    }
    
    // Step 1. Create environment
    Minotaur::EnvPtr env = new Minotaur::Environment();
    std::cout << "Environment created." << std::endl;
    
    // Step 2. Read problem from .nl file using AMPL interface
    MINOTAUR_AMPL::AMPLInterface* iface = new MINOTAUR_AMPL::AMPLInterface(env);
    Minotaur::ProblemPtr p = iface->readInstance(argv[1]);
    
    if (!p) {
        std::cerr << "Error reading problem from " << argv[1] << std::endl;
        delete iface;
        delete env;
        return 1;
    }
    
    std::cout << "Problem loaded successfully." << std::endl;
    std::cout << "Number of variables: " << p->getNumVars() << std::endl;
    std::cout << "Number of constraints: " << p->getNumCons() << std::endl;
    
    // Check if problem has integer variables
    int num_int_vars = 0;
    for (Minotaur::VariableConstIterator vit = p->varsBegin(); vit != p->varsEnd(); ++vit) {
        if ((*vit)->getType() == Minotaur::Binary || (*vit)->getType() == Minotaur::Integer) {
            num_int_vars++;
        }
    }
    std::cout << "Number of integer variables: " << num_int_vars << std::endl;
    
    // Step 3. Create relaxation
    std::cout << "Creating relaxation..." << std::endl;
    Minotaur::RelaxationPtr rel = (Minotaur::RelaxationPtr) new Minotaur::Relaxation(p, env);
    std::cout << "Relaxation created." << std::endl;
    
    // Step 4. Create and initialize NLP engines
    std::cout << "Creating NLP engine..." << std::endl;
    Minotaur::EnginePtr nlp_engine = new Minotaur::IpoptEngine(env);
    std::cout << "NLP engine created." << std::endl;
    
    std::cout << "Loading problem into NLP engine..." << std::endl;
    nlp_engine->load(rel);
    std::cout << "NLP engine loaded successfully." << std::endl;
    
    // Step 5. Create MIP engine (for rounding step)
    std::cout << "Creating MIP engine..." << std::endl;
    Minotaur::EnginePtr mip_engine = new Minotaur::IpoptEngine(env);
    mip_engine->load(p);
    std::cout << "MIP engine loaded successfully." << std::endl;
    
    // Step 6. Create Feasibility Pump object
    std::cout << "Creating Feasibility Pump object..." << std::endl;
    std::cout << "About to call FeasibilityPump constructor..." << std::endl;
    
    Minotaur::FeasibilityPump* fp = nullptr;
    try {
        fp = new Minotaur::FeasibilityPump(env, p, nlp_engine, mip_engine);
        std::cout << "Feasibility Pump created successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception during FP creation: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception during FP creation" << std::endl;
        return 1;
    }
    
    if (!fp) {
        std::cerr << "FeasibilityPump pointer is null!" << std::endl;
        return 1;
    }
    
    // Step 7. Create solution pool
    std::cout << "Creating solution pool..." << std::endl;
    Minotaur::SolutionPoolPtr s_pool = new Minotaur::SolutionPool(env, p);
    std::cout << "Solution pool created." << std::endl;
    
    // Step 8. Try to call solve
    std::cout << "About to call fp->solve()..." << std::endl;
    std::cout << "Checking if solve method exists..." << std::endl;
    
    try {
        // Try with all nullptrs first
        std::cout << "Calling solve with nullptr arguments..." << std::endl;
        fp->solve(nullptr, nullptr, s_pool);
        std::cout << "Solve completed without crash!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception during solve: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception during solve" << std::endl;
        return 1;
    }
    
    // Step 9. Print results
    std::cout << "Feasibility Pump completed.\n";
    fp->writeStats(std::cout);
    
    // Cleanup
    delete fp;
    delete iface;
    delete env;
    
    return 0;
}
