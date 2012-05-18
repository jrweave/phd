include ../../Makefile.inc

SUBDIR	= rdf/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testRDFTerm testRDFDictionary testNTriplesReader

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
	$(ECHO) $(CC) $(CFLAGS) -o testRDFTerm testRDFTerm.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(CC) $(CFLAGS) -o testRDFTerm testRDFTerm.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(ECHO) [TEST] ./testRDFTerm
	./testRDFTerm

testRDFDictionary : testRDFDictionary.cpp ../RDFDictionary.h ../RDFDictionary-inl.h
	$(ECHO) running test $(SUBDIR)/testRDFDictionary
	$(ECHO) $(CC) $(CFLAGS) -o testRDFDictionary testRDFDictionary.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(CC) $(CFLAGS) -o testRDFDictionary testRDFDictionary.cpp ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o
	$(ECHO) [TEST] ./testRDFDictionary
	./testRDFDictionary

testNTriplesReader : testNTriplesReader.cpp ../NTriplesReader.h
	$(ECHO) running test $(SUBDIR)/testNTriplesReader
	$(ECHO) $(CC) $(CFLAGS) -o testNTriplesReader testNTriplesReader.cpp ../NTriplesReader.o  ../RDFTriple.o ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../io/IFStream.o ../../io/IOException.o ../../io/InputStream.o
	$(CC) $(CFLAGS) -o testNTriplesReader testNTriplesReader.cpp ../NTriplesReader.o ../RDFTriple.o ../RDFTerm.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../io/IFStream.o ../../io/IOException.o ../../io/InputStream.o
	$(ECHO) [TEST] ./testNTriplesReader
	./testNTriplesReader
