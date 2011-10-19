include ../../Makefile.inc

SUBDIR	= ex/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testPtr testDPtr testOPtr testMPtr

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

testDPtr : testDPtr.cpp ../DPtr.h ../DPtr-inl.h testPtr
	$(ECHO) $(CC) $(CFLAGS) -o testDPtr testDPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(CC) $(CFLAGS) -o testDPtr testDPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(ECHO) [TEST] ./testDPtr
	./testDPtr

testOPtr : testOPtr.cpp ../OPtr.h ../OPtr-inl.h testDPtr
	$(ECHO) $(CC) $(CFLAGS) -o testOPtr testOPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(CC) $(CFLAGS) -o testOPtr testOPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(ECHO) [TEST] ./testOPtr
	./testOPtr

testMPtr : testMPtr.cpp ../MPtr.h ../MPtr-inl.h testDPtr
	$(ECHO) $(CC) $(CFLAGS) -o testMPtr testMPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(CC) $(CFLAGS) -o testMPtr testMPtr.cpp ../Ptr.o ../BadAllocException.o ../../ex/TraceableException.o
	$(ECHO) [TEST] ./testMPtr
	./testMPtr
