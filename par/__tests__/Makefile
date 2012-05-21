include ../../Makefile.inc

SUBDIR	= par/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testMPIDelimFileInputStream testMPIPacketDistributor testStringDistributor

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

testMPIDelimFileInputStream : testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o
	$(ECHO) running test $(SUBDIR)/testMPIDelimFileInputStream
	$(ECHO) $(CC) $(CFLAGS) -o testMPIDelimFileInputStream testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(CC) $(CFLAGS) -o testMPIDelimFileInputStream testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(ECHO) [TEST] ./testMPIDelimFileInputStream $(TESTFILE)
	$(RUN) -np 4 ./testMPIDelimFileInputStream $(TESTFILE)

testMPIPacketDistributor : testMPIPacketDistributor.cpp ../MPIPacketDistributor.o
	$(ECHO) running test $(SUBDIR)/testMPIPacketDistributor
	$(ECHO) $(CC) $(CFLAGS) -o testMPIPacketDistributor testMPIPacketDistributor.cpp ../MPIPacketDistributor.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o
	$(CC) $(CFLAGS) -o testMPIPacketDistributor testMPIPacketDistributor.cpp ../MPIPacketDistributor.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o
	$(ECHO) [TEST] ./testMPIPacketDistributor
	$(RUN) -np 4 ./testMPIPacketDistributor

testStringDistributor : testStringDistributor.cpp ../StringDistributor.o foaf.nt
	$(ECHO) running test $(SUBDIR)/testStringDistributor
	$(ECHO) $(CC) $(CFLAGS) -o testStringDistributor testStringDistributor.cpp ../StringDistributor.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o
	$(CC) $(CFLAGS) -o testStringDistributor testStringDistributor.cpp ../StringDistributor.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o
	$(ECHO) [TEST] ./testStringDistributor `pwd`/foaf.nt
	$(RUN) -np 4 ./testStringDistributor `pwd`/foaf.nt
