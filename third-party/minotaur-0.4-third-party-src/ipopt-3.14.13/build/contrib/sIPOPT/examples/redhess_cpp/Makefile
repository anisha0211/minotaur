# Copyright (C) 2010 Hans Pirnay
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

##########################################################################
#    You can modify this example makefile to fit for your own program.   #
#    Usually, you only need to change the four CHANGEME entries below.   #
##########################################################################

# CHANGEME: This should be the name of your executable
EXE = redhess_cpp

# CHANGEME: Here is the name of all object files corresponding to the source
#           code that you wrote in order to define the problem statement
OBJS =  redhess_cpp.o \
	MySensTNLP.o

# CHANGEME: Additional libraries
ADDLIBS =

# CHANGEME: Additional flags for compilation (e.g., include flags)
ADDINCFLAGS =

##########################################################################
#  Usually, you don't have to change anything below.  Note that if you   #
#  change certain compiler options, you might have to recompile Ipopt.   #
##########################################################################

# C++ Compiler command
CXX = /home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/bin/x86_64-conda-linux-gnu-c++

# C++ Compiler options
CXXFLAGS = -fvisibility-inlines-hidden -fmessage-length=0 -march=nocona -mtune=haswell -ftree-vectorize -fPIC -fstack-protector-strong -fno-plt -O2 -ffunction-sections -pipe -isystem /home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/include

# additional C++ Compiler options for linking
CXXLINKFLAGS =  -Wl,--rpath -Wl,/home/mech/btech/me1222013/minotaur/third-party/lib

prefix=/home/mech/btech/me1222013/minotaur/third-party
exec_prefix=${prefix}

# TODO there should be an sipopt .pc file from which to get flags

# Include directories
INCL = `PKG_CONFIG_PATH=/home/mech/btech/me1222013/minotaur/third-party/lib/pkgconfig: x86_64-conda-linux-gnu-pkg-config --cflags ipopt` $(ADDINCFLAGS)
#INCL = -I${prefix}/include/coin -I/home/mech/btech/me1222013/minotaur/third-party/include/coin-or/mumps   -DIPOPTLIB_BUILD $(ADDINCFLAGS)

# Linker flags
LIBS = `PKG_CONFIG_PATH=/home/mech/btech/me1222013/minotaur/third-party/lib/pkgconfig: x86_64-conda-linux-gnu-pkg-config --libs ipopt` -lsipopt
#LIBS = -L${exec_prefix}/lib -lsipopt -lipopt -L/home/mech/btech/me1222013/minotaur/third-party/lib -L/home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/lib -lcoinmumps -llapack -lblas   -lm  -ldl

all: $(EXE)

.SUFFIXES: .cpp .o

$(EXE): $(OBJS)
	$(CXX) $(CXXLINKFLAGS) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) $(ADDLIBS)

clean:
	rm -rf $(EXE) $(OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCL) -c -o $@ $<
