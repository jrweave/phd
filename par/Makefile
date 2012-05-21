include ../Makefile.inc

SUBDIR	= par
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= MPIFileInputStream.o MPIDelimFileInputStream.o DistException.o MPIPacketDistributor.o StringDistributor.o

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
