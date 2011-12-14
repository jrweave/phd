include ../Makefile.inc

SUBDIR	= ex
CFLAGS  = $(PRJCFLAGS) -I..
OBJS		= TraceableException.o

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

TraceableException.o : TraceableException.h TraceableException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o TraceableException.o TraceableException.cpp
	$(CC) $(CFLAGS) -c -o TraceableException.o TraceableException.cpp
