include ../Makefile.inc

SUBDIR	= iri
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= MalformedIRIRefException.o IRIRef.o

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

MalformedIRIRefException.o : MalformedIRIRefException.h MalformedIRIRefException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MalformedIRIRefException.o MalformedIRIRefException.cpp
	$(CC) $(CFLAGS) -c -o MalformedIRIRefException.o MalformedIRIRefException.cpp

IRIRef.o : IRIRef.h IRIRef-inl.h IRIRef.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o IRIRef.o IRIRef.cpp
	$(CC) $(CFLAGS) -c -o IRIRef.o IRIRef.cpp
