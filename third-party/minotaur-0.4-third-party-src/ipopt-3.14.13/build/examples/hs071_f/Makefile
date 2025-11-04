# Copyright (C) 2003, 2010 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

##########################################################################
#    You can modify this example makefile to fit for your own program.   #
#    Usually, you only need to change the four CHANGEME entries below.   #
##########################################################################

# CHANGEME: This should be the name of your executable
EXE = hs071_f

# CHANGEME: Here is the name of all object files corresponding to the source
#           code that you wrote in order to define the problem statement
OBJS = hs071_f.o
#OBJS = hs071_fs.o

# CHANGEME: Additional libraries
ADDLIBS =

# CHANGEME: Additional flags for compilation (e.g., include flags)
ADDINCFLAGS =

##########################################################################
#  Usually, you don't have to change anything below.  Note that if you   #
#  change certain compiler options, you might have to recompile Ipopt.   #
##########################################################################

# Fortran Compiler options
F77 = /home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/bin/x86_64-conda-linux-gnu-gfortran

# Fotran Compiler options
FFLAGS = -march=nocona -mtune=haswell -ftree-vectorize -fPIC -fstack-protector-strong -fno-plt -O2 -ffunction-sections -pipe -isystem /home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/include -I/home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/include

# additional Fortran Compiler options for linking
F77LINKFLAGS =  -Wl,--rpath -Wl,/home/mech/btech/me1222013/minotaur/third-party/lib

prefix=/home/mech/btech/me1222013/minotaur/third-party
exec_prefix=${prefix}

# Include directories
INCL = `PKG_CONFIG_PATH=/home/mech/btech/me1222013/minotaur/third-party/lib/pkgconfig: x86_64-conda-linux-gnu-pkg-config --cflags ipopt` $(ADDINCFLAGS)
#INCL = -I${prefix}/include/coin-or -I/home/mech/btech/me1222013/minotaur/third-party/include/coin-or/mumps   -DIPOPTLIB_BUILD $(ADDINCFLAGS)

# Linker flags
LIBS = `PKG_CONFIG_PATH=/home/mech/btech/me1222013/minotaur/third-party/lib/pkgconfig: x86_64-conda-linux-gnu-pkg-config --libs ipopt` -lstdc++ -lm
#LIBS = -L${exec_prefix}/lib -lipopt -L/home/mech/btech/me1222013/minotaur/third-party/lib -L/home/mech/btech/me1222013/miniconda3/envs/minotaur-dev/lib -lcoinmumps -llapack -lblas   -lm  -ldl -lstdc++ -lm

all: $(EXE)

.SUFFIXES: .f .o

$(EXE): $(OBJS)
	$(F77) $(F77LINKFLAGS) $(FFLAGS) -o $@ $(OBJS) $(ADDLIBS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS) IPOPT.OUT

.f.o:
	$(F77) $(FFLAGS) $(INCL) -c -o $@ $<
