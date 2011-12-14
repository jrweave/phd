include ../../Makefile.inc

SUBDIR	= lang/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testLangTag testLangRange

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

thorough : runtests

runtests : $(TESTS)

force_look :
	true

testLangTag : testLangTag.cpp ../LangTag.o
	$(ECHO) running test $(SUBDIR)/testLangTag
	$(ECHO) $(CC) $(CFLAGS) -o testLangTag testLangTag.cpp ../LangTag.o ../../ptr/SizeUnknownException.o ../MalformedLangTagException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ptr/alloc.o
	$(CC) $(CFLAGS) -o testLangTag testLangTag.cpp ../LangTag.o ../../ptr/SizeUnknownException.o ../MalformedLangTagException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ptr/alloc.o
	$(ECHO) [TEST] ./testLangTag
	./testLangTag

testLangRange : testLangRange.cpp ../LangRange.o ../LangTag.o
	$(ECHO) running test $(SUBDIR)/testLangRange
	$(ECHO) $(CC) $(CFLAGS) -o testLangRange testLangRange.cpp ../LangRange.o ../LangTag.o ../../ptr/SizeUnknownException.o ../MalformedLangTagException.o ../MalformedLangRangeException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ptr/alloc.o
	$(CC) $(CFLAGS) -o testLangRange testLangRange.cpp ../LangRange.o ../LangTag.o ../../ptr/SizeUnknownException.o ../MalformedLangTagException.o ../MalformedLangRangeException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ptr/alloc.o
	$(ECHO) [TEST] ./testLangRange
	./testLangRange
