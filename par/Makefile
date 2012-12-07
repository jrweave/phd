# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

include ../Makefile.inc

SUBDIR	= par
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= DistException.o StringDistributor.o DistComputation.o
ifeq ($(USE_PAR_MPI), yes)
OBJS		+= MPIFileInputStream.o MPIDelimFileInputStream.o MPIPacketDistributor.o MPIFileOutputStream.o MPIDistPtrFileOutputStream.o
endif

all : build __tests__

thorough : runtests
	cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(OBJS)

prep : 

__tests__ : force_look
	$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	$(ECHO) running tests for $(SUBDIR) : $(MAKE) $(MFLAGS) runtests
	cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	-$(RM) -vf $(OBJS) 
	-$(RM) -vfr *.dSYM
	cd __tests__; $(MAKE) $(MFLAGS) clean

distclean : clean

force_look :
	true

MPIFileInputStream.o : MPIFileInputStream.h MPIFileInputStream-inl.h MPIFileInputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MPIFileInputStream.o MPIFileInputStream.cpp
	$(CC) $(CFLAGS) -c -o MPIFileInputStream.o MPIFileInputStream.cpp

MPIDelimFileInputStream.o : MPIDelimFileInputStream.h MPIDelimFileInputStream-inl.h MPIDelimFileInputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MPIDelimFileInputStream.o MPIDelimFileInputStream.cpp
	$(CC) $(CFLAGS) -c -o MPIDelimFileInputStream.o MPIDelimFileInputStream.cpp

DistException.o : DistException.h DistException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o DistException.o DistException.cpp
	$(CC) $(CFLAGS) -c -o DistException.o DistException.cpp

MPIPacketDistributor.o : MPIPacketDistributor.h MPIPacketDistributor.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MPIPacketDistributor.o MPIPacketDistributor.cpp
	$(CC) $(CFLAGS) -c -o MPIPacketDistributor.o MPIPacketDistributor.cpp

StringDistributor.o : StringDistributor.h StringDistributor.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o StringDistributor.o StringDistributor.cpp
	$(CC) $(CFLAGS) -c -o StringDistributor.o StringDistributor.cpp

MPIFileOutputStream.o : MPIFileOutputStream.h MPIFileOutputStream-inl.h MPIFileOutputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MPIFileOutputStream.o MPIFileOutputStream.cpp
	$(CC) $(CFLAGS) -c -o MPIFileOutputStream.o MPIFileOutputStream.cpp

MPIDistPtrFileOutputStream.o : MPIDistPtrFileOutputStream.h MPIDistPtrFileOutputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MPIDistPtrFileOutputStream.o MPIDistPtrFileOutputStream.cpp
	$(CC) $(CFLAGS) -c -o MPIDistPtrFileOutputStream.o MPIDistPtrFileOutputStream.cpp

DistComputation.o : DistComputation.h DistComputation.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o DistComputation.o DistComputation.cpp
	$(CC) $(CFLAGS) -c -o DistComputation.o DistComputation.cpp
