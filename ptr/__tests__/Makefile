include ../../Makefile.inc

SUBDIR	= ex/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testPtr

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testPtr : testPtr.cpp ../Ptr.h ../Ptr.cpp ../Ptr.o ../BadAllocException.h ../BadAllocException.cpp ../BadAllocException.o ../../ex/TraceableException.h ../../ex/TraceableException.cpp ../../ex/TraceableException.o
	$(ECHO) $(CC) $(CFLAGS) -o testPtr testPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(CC) $(CFLAGS) -o testPtr testPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(ECHO) [TEST] ./testPtr
	./testPtr
