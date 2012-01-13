include ../../Makefile.inc

SUBDIR	= rdf/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testRDFTerm

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

testRDFTerm : testRDFTerm.cpp ../RDFTerm.o
	$(ECHO) running test $(SUBDIR)/testRDFTerm
	$(ECHO) $(CC) $(CFLAGS) -o testRDFTerm testRDFTerm.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../ucs/UCSIter.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(CC) $(CFLAGS) -o testRDFTerm testRDFTerm.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../ucs/UCSIter.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(ECHO) [TEST] ./testRDFTerm
	./testRDFTerm
