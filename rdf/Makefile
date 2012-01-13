include ../Makefile.inc

SUBDIR	= rdf
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= RDFTerm.o

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
