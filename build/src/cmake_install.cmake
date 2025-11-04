# Install script for directory: /home/mech/btech/me1222013/minotaur/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ".")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES
    "/home/mech/btech/me1222013/minotaur/src/base/MinotaurDeconfig.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ActiveNodeStore.h"
    "/home/mech/btech/me1222013/minotaur/src/base/AnalyticalCenter.h"
    "/home/mech/btech/me1222013/minotaur/src/base/BndProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Branch.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Brancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/BranchAndBound.h"
    "/home/mech/btech/me1222013/minotaur/src/base/BrCand.h"
    "/home/mech/btech/me1222013/minotaur/src/base/BrVarCand.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CGraph.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CNode.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ConBoundMod.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Constraint.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CoverCutGenerator.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CutInfo.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CutManager.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CxQuadHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/CxUnivarHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Eigen.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Engine.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Environment.h"
    "/home/mech/btech/me1222013/minotaur/src/base/FeasibilityPump.h"
    "/home/mech/btech/me1222013/minotaur/src/base/FixVarsHeur.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Exception.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Function.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Handler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/HessianOfLag.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Heuristic.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Iterate.h"
    "/home/mech/btech/me1222013/minotaur/src/base/IntVarHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Jacobian.h"
    "/home/mech/btech/me1222013/minotaur/src/base/KnapsackList.h"
    "/home/mech/btech/me1222013/minotaur/src/base/KnapCovHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LexicoBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinearCut.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinearFunction.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinearHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinFeasPump.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Linearizations.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinBil.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinConMod.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LinMods.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LGCIGenerator.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Logger.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LPEngine.h"
    "/home/mech/btech/me1222013/minotaur/src/base/LPRelaxation.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MaxFreqBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MaxInfBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MaxVioBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MILPEngine.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MINLPDiving.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Modification.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MsProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/MultilinearTermsHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NLPEngine.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NLPRelaxation.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NlPresHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NLPMultiStart.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NlWriter.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Node.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NodeHeap.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NodeRelaxer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NodeIncRelaxer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NodeProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NodeStack.h"
    "/home/mech/btech/me1222013/minotaur/src/base/NonlinearFunction.h"
    "/home/mech/btech/me1222013/minotaur/src/base/OAHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Objective.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Operations.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Option.h"
    "/home/mech/btech/me1222013/minotaur/src/base/OpCode.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParBndProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParBranchAndBound.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParCutMan.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParMINLPDiving.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParNodeIncRelaxer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParQGBranchAndBound.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParQGHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParQGHandlerAdvance.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParPCBProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParReliabilityBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ParTreeManager.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PCBProcessor.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PerspCon.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PerspCutGenerator.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PolynomialFunction.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PreAuxVars.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PreDelVars.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PreMod.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Presolver.h"
    "/home/mech/btech/me1222013/minotaur/src/base/PreSubstVars.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Problem.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ProblemSize.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ProbStructure.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QPEngine.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QGHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QGHandlerAdvance.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QuadHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/kPowHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QuadraticFunction.h"
    "/home/mech/btech/me1222013/minotaur/src/base/QuadTransformer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/RandomBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/RCHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Relaxation.h"
    "/home/mech/btech/me1222013/minotaur/src/base/ReliabilityBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SecantMod.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SimpleCutMan.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SimpleTransformer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SimplexQuadCutGen.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Solution.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SolutionPool.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SOS.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SOS1Handler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SOS2Handler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SOSBrCand.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SppHeur.h"
    "/home/mech/btech/me1222013/minotaur/src/base/STOAHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/HybridBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Timer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Transformer.h"
    "/home/mech/btech/me1222013/minotaur/src/base/TransPoly.h"
    "/home/mech/btech/me1222013/minotaur/src/base/TransSep.h"
    "/home/mech/btech/me1222013/minotaur/src/base/TreeManager.h"
    "/home/mech/btech/me1222013/minotaur/src/base/SamplingHeur.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Types.h"
    "/home/mech/btech/me1222013/minotaur/src/base/UnivarQuadHandler.h"
    "/home/mech/btech/me1222013/minotaur/src/base/UnambRelBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/WeakBrancher.h"
    "/home/mech/btech/me1222013/minotaur/src/base/VarBoundMod.h"
    "/home/mech/btech/me1222013/minotaur/src/base/WarmStart.h"
    "/home/mech/btech/me1222013/minotaur/src/base/Variable.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqBivar.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqCGs.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqLFs.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqMonomial.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqUCGs.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqUnivar.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqVars.h"
    "/home/mech/btech/me1222013/minotaur/src/base/YEqQfBil.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mech/btech/me1222013/minotaur/build/src/libminotaur.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mech/btech/me1222013/minotaur/src/interfaces/EngineFactory.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mech/btech/me1222013/minotaur/build/src/base/MinotaurConfig.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mech/btech/me1222013/minotaur/build/src/base/MinotaurCFortran.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mech/btech/me1222013/minotaur/build/src/base/Version.h")
endif()

