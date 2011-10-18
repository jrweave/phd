include ../Makefile.inc

CFLAGS  = $(PRJCFLAGS) -I..
OBJS		= TraceableException.o

all : $(OBJS) __tests__

__tests__ : force_look
	$(ECHO) looking into ex/__tests__ : $(MAKE) $(MFLAGS)
	cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	$(ECHO) running tests for ex : $(MAKE) $(MFLAGS) runtests
	cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in ex
	-$(RM) -vf $(OBJS) 
	-$(RM) -vfr *.dSYM
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

TraceableException.o : TraceableException.h TraceableException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o TraceableException.o TraceableException.cpp
	$(CC) $(CFLAGS) -c -o TraceableException.o TraceableException.cpp
