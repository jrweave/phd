include ../Makefile.inc

SUBDIR	= rdf
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= RDFTerm.o RDFTriple.o NTriplesReader.o NTriplesWriter.o

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
	-$(RM) -vf rdf_constants.*
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

RDFTerm.o : RDFTerm.h RDFTerm-inl.h RDFTerm.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o RDFTerm.o RDFTerm.cpp
	$(CC) $(CFLAGS) -c -o RDFTerm.o RDFTerm.cpp

RDFTriple.o : RDFTriple.h RDFTriple-inl.h RDFTriple.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o RDFTriple.o RDFTriple.cpp
	$(CC) $(CFLAGS) -c -o RDFTriple.o RDFTriple.cpp

NTriplesReader.o : NTriplesReader.h NTriplesReader.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o NTriplesReader.o NTriplesReader.cpp
	$(CC) $(CFLAGS) -c -o NTriplesReader.o NTriplesReader.cpp

NTriplesWriter.o : NTriplesWriter.h NTriplesWriter.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o NTriplesWriter.o NTriplesWriter.cpp
	$(CC) $(CFLAGS) -c -o NTriplesWriter.o NTriplesWriter.cpp
