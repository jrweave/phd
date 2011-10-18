include ../../Makefile.inc

CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testTraceableException

all : $(TESTS)

clean :
	$(ECHO) cleaning up in ex/__tests__
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : all
	$(ECHO) ./testTraceableException
	./testTraceableException

force_look :
	true

testTraceableException : testTraceableException.cpp ../TraceableException.h ../TraceableException.cpp ../TraceableException.o
	$(ECHO) $(CC) $(CFLAGS) -o testTraceableException testTraceableException.cpp ../TraceableException.o
	$(CC) $(CFLAGS) -o testTraceableException testTraceableException.cpp ../TraceableException.o
