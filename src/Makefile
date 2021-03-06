##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Tue Feb 19 21:16:26 PST 2013
## Last Modified: Fri Feb 22 16:41:55 PST 2013
## Filename:      ...smake/Makefile
## Syntax:        GNU Makefile
##
## Description: This Makefile creates the smake program for multiple 
##              operating systems.  Also, gives basic guidelines 
##              on how to compile for Windows using MinGW in linux.
##

# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.4" in Apple OS X to compile for OS X 10.4 and later (for example,
# you can compile for OS X 10.4 computers even if you are using a 10.7
# computer to compile the code).
ENV = 

# Set the ARCH variable to compile 32-bit executable on 64-bit computer
# (if you have 32-bit libraries installed on your 64-bit computer).
# See the examples for ARCH on OS X and linux given in comments below.
ARCH = 

# Select options based on the Operating system type
ifeq ($(origin OSTYPE), undefined)
   ifeq ($(shell uname),Darwin)
      OSTYPE = OSX
      ENV = MACOSX_DEPLOYMENT_TARGET=10.10
      # use the following to compile for 32-bit architecture on 64-bit comps:
      # ARCH = -m32 -arch i386
   else
      OSTYPE = LINUX
      # use the following to compile for 32-bit architecture on 64-bit comps:
      # (you will need 32-bit libraries in order to do this)
      # ARCH = -m32 
   endif
endif

CPP = Options.cpp

COMPILER = LANG=C g++
PREFLAGS = -O3

# using C++ 2011 standard:
PREFLAGS += -std=c++11


# MinGW compiling setup (used to compile for Microsoft Windows but actual
# compiling can be done in Linux). You have to install MinGW and this
# variable will probably have to be changed to the correct path to the
# MinGW compiler:
# COMPILER = /usr/bin/i686-pc-mingw32-g++

.PHONY: smake

all: smake

smake:
	$(ENV) $(COMPILER) $(ARCH) $(PREFLAGS) -o smake $(CPP) smake.cpp

install:
	sudo cp smake /usr/local/bin/

clean:
	-rm smake



