include ../../Makefile.inc

SUBDIR	= iri/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testIRIRef

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

testIRIRef : testIRIRef.cpp ../IRIRef.o
	$(ECHO) running test $(SUBDIR)/testIRIRef
	$(ECHO) $(CC) $(CFLAGS) -o testIRIRef testIRIRef.cpp ../IRIRef.o ../../ptr/SizeUnknownException.o ../MalformedIRIRefException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../ucs/utf.o ../../ucs/UCSIter.o ../../ucs/InvalidCodepointException.o ../../ucs/InvalidEncodingException.o ../../sys/endian.o ../../ptr/alloc.o
	$(CC) $(CFLAGS) -o testIRIRef testIRIRef.cpp ../IRIRef.o ../../ptr/SizeUnknownException.o ../MalformedIRIRefException.o ../../ex/TraceableException.o ../../ptr/Ptr.o ../../ptr/BadAllocException.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../ucs/utf.o ../../ucs/UCSIter.o ../../ucs/InvalidCodepointException.o ../../ucs/InvalidEncodingException.o ../../sys/endian.o ../../ptr/alloc.o
	$(ECHO) [TEST] ./testIRIRef
	./testIRIRef
