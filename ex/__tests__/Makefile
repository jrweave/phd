include ../../Makefile.inc

CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testTraceableException testBaseException

all :

clean :
	$(ECHO) cleaning up in ex/__tests__
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testTraceableException : testTraceableException.cpp ../TraceableException.h ../TraceableException.cpp ../TraceableException.o
	$(ECHO) $(CC) $(CFLAGS) -o testTraceableException testTraceableException.cpp ../TraceableException.o
	$(CC) $(CFLAGS) -o testTraceableException testTraceableException.cpp ../TraceableException.o
	$(ECHO) ./testTraceableException
	./testTraceableException

testBaseException : testBaseException.cpp ../BaseException.h ../BaseException-inl.h testTraceableException
	$(ECHO) $(CC) $(CFLAGS) -o testBaseException testBaseException.cpp ../TraceableException.o
	$(CC) $(CFLAGS) -o testBaseException testBaseException.cpp ../TraceableException.o
	$(ECHO) ./testBaseException
	./testBaseException
