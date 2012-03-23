include ../../Makefile.inc

SUBDIR	= rif/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testRIFConst testRIFVar testRIFTerm testRIFAtomic testRIFCondition testRIFDictionary

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

testRIFConst : testRIFConst.cpp ../RIFConst.o
	$(ECHO) running test $(SUBDIR)/testRIFConst
	$(ECHO) $(CC) $(CFLAGS) -o testRIFConst testRIFConst.cpp ../RIFConst.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o
	$(CC) $(CFLAGS) -o testRIFConst testRIFConst.cpp ../RIFConst.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o
	$(ECHO) [TEST] ./testRIFConst
	./testRIFConst

testRIFVar : testRIFVar.cpp ../RIFVar.o
	$(ECHO) running test $(SUBDIR)/testRIFVar
	$(ECHO) $(CC) $(CFLAGS) -o testRIFVar testRIFVar.cpp ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(CC) $(CFLAGS) -o testRIFVar testRIFVar.cpp ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(ECHO) [TEST] ./testRIFVar
	./testRIFVar

testRIFTerm : testRIFTerm.cpp ../RIFTerm.o
	$(ECHO) running test $(SUBDIR)/testRIFTerm
	$(ECHO) $(CC) $(CFLAGS) -o testRIFTerm testRIFTerm.cpp ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(CC) $(CFLAGS) -o testRIFTerm testRIFTerm.cpp ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(ECHO) [TEST] ./testRIFTerm
	./testRIFTerm

testRIFAtomic : testRIFAtomic.cpp ../RIFAtomic.o
	$(ECHO) running test $(SUBDIR)/testRIFAtomic
	$(ECHO) $(CC) $(CFLAGS) -o testRIFAtomic testRIFAtomic.cpp ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(CC) $(CFLAGS) -o testRIFAtomic testRIFAtomic.cpp ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(ECHO) [TEST] ./testRIFAtomic
	./testRIFAtomic

testRIFCondition : testRIFCondition.cpp ../RIFCondition.o
	$(ECHO) running test $(SUBDIR)/testRIFCondition
	$(ECHO) $(CC) $(CFLAGS) -o testRIFCondition testRIFCondition.cpp ../RIFCondition.o ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(CC) $(CFLAGS) -o testRIFCondition testRIFCondition.cpp ../RIFCondition.o ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(ECHO) [TEST] ./testRIFCondition
	./testRIFCondition

testRIFDictionary : testRIFDictionary.cpp ../RIFDictionary.o
	$(ECHO) running test $(SUBDIR)/testRIFDictionary
	$(ECHO) $(CC) $(CFLAGS) -o testRIFDictionary testRIFDictionary.cpp ../RIFDictionary.o ../RIFCondition.o ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(CC) $(CFLAGS) -o testRIFDictionary testRIFDictionary.cpp ../RIFDictionary.o ../RIFCondition.o ../RIFAtomic.o ../RIFTerm.o ../RIFVar.o ../../iri/IRIRef.o ../../lang/LangTag.o ../../ptr/Ptr.o ../../ptr/SizeUnknownException.o ../../ex/TraceableException.o ../../ucs/utf.o ../../ucs/nf.o ../../lang/MalformedLangTagException.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidEncodingException.o ../../ptr/BadAllocException.o ../../sys/endian.o ../../ucs/InvalidCodepointException.o ../../ptr/alloc.o ../../ucs/UTF8Iter.o ../../rdf/RDFTerm.o ../../rif/RIFConst.o
	$(ECHO) [TEST] ./testRIFDictionary
	./testRIFDictionary