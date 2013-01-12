# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

include ../../Makefile.inc

SUBDIR	= par/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		=
ifeq ($(USE_PAR_MPI), yes)
TESTS		+= testMPIDelimFileInputStream testMPIPacketDistributor testStringDistributor testMPIDistPtrFileOutputStream testDistRDFDictEncode testMPIPartialFileInputStream
endif

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM
	-$(RM) -vf foaf.out

thorough : runtests

runtests : $(TESTS)

force_look :
	true

testMPIDelimFileInputStream : testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o
	$(ECHO) running test $(SUBDIR)/testMPIDelimFileInputStream
	$(ECHO) $(CC) $(CFLAGS) -o testMPIDelimFileInputStream testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(CC) $(CFLAGS) -o testMPIDelimFileInputStream testMPIDelimFileInputStream.cpp ../MPIDelimFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(ECHO) [TEST] ./testMPIDelimFileInputStream _test_
	$(RUN) -np 4 ./testMPIDelimFileInputStream _test_

testMPIPartialFileInputStream : testMPIPartialFileInputStream.cpp ../MPIPartialFileInputStream.o
	$(ECHO) running test $(SUBDIR)/testMPIPartialFileInputStream
	$(ECHO) $(CC) $(CFLAGS) -o testMPIPartialFileInputStream testMPIPartialFileInputStream.cpp ../MPIPartialFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(CC) $(CFLAGS) -o testMPIPartialFileInputStream testMPIPartialFileInputStream.cpp ../MPIPartialFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../MPIFileInputStream.o ../../io/IOException.o ../../io/InputStream.o
	$(ECHO) [TEST] ./testMPIPartialFileInputStream `pwd`/foaf.nt `pwd`/foaf-1000-11000.txt
	$(RUN) -np 4 ./testMPIPartialFileInputStream `pwd`/foaf.nt `pwd`/foaf-1000-11000.txt

testMPIPacketDistributor : testMPIPacketDistributor.cpp ../MPIPacketDistributor.o
	$(ECHO) running test $(SUBDIR)/testMPIPacketDistributor
	$(ECHO) $(CC) $(CFLAGS) -o testMPIPacketDistributor testMPIPacketDistributor.cpp ../MPIPacketDistributor.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o
	$(CC) $(CFLAGS) -o testMPIPacketDistributor testMPIPacketDistributor.cpp ../MPIPacketDistributor.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o
	$(ECHO) [TEST] ./testMPIPacketDistributor
	$(RUN) -np 4 ./testMPIPacketDistributor

testStringDistributor : testStringDistributor.cpp ../StringDistributor.o foaf.nt
	$(ECHO) running test $(SUBDIR)/testStringDistributor
	$(ECHO) $(CC) $(CFLAGS) -o testStringDistributor testStringDistributor.cpp ../StringDistributor.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../../sys/endian.o
	$(CC) $(CFLAGS) -o testStringDistributor testStringDistributor.cpp ../StringDistributor.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../../sys/endian.o
	$(ECHO) [TEST] ./testStringDistributor `pwd`/foaf.nt
	$(RUN) -np 4 ./testStringDistributor `pwd`/foaf.nt

testMPIDistPtrFileOutputStream : testMPIDistPtrFileOutputStream.cpp ../MPIDistPtrFileOutputStream.o foaf.nt ../MPIFileOutputStream.cpp ../MPIFileOutputStream.o
	-$(RM) -vf foaf.out
	$(ECHO) running test $(SUBDIR)/testMPIDistPtrFileOutputStream
	$(ECHO) $(CC) $(CFLAGS) -o testMPIDistPtrFileOutputStream testMPIDistPtrFileOutputStream.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../../sys/endian.o
	$(CC) $(CFLAGS) -o testMPIDistPtrFileOutputStream testMPIDistPtrFileOutputStream.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../../sys/endian.o
	$(ECHO) [TEST] ./testMPIDistPtrFileOutputStream `pwd`/foaf.nt `pwd`/foaf.out
	$(RUN) -np 4 ./testMPIDistPtrFileOutputStream `pwd`/foaf.nt `pwd`/foaf.out

testDistRDFDictEncode : testDistRDFDictEncode.cpp ../MPIDistPtrFileOutputStream.o foaf.nt ../MPIFileOutputStream.cpp ../MPIFileOutputStream.o
	-$(RM) -vf foaf.out
	$(ECHO) running test $(SUBDIR)/testDistRDFDictEncode
	$(ECHO) $(CC) $(CFLAGS) -o testDistRDFDictEncode testDistRDFDictEncode.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../DistComputation.o ../StringDistributor.o ../../sys/endian.o
	$(CC) $(CFLAGS) -o testDistRDFDictEncode testDistRDFDictEncode.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../DistComputation.o ../StringDistributor.o ../../sys/endian.o
	$(ECHO) [TEST] ./testDistRDFDictEncode `pwd`/foaf.nt `pwd`/foaf.out
	$(RUN) -np 4 ./testDistRDFDictEncode `pwd`/foaf.nt `pwd`/foaf.out
	$(ECHO) $(CC) $(CFLAGS) -DTESTFILE='"foaf_1.nt"' -DNUMLINES=1 -o testDistRDFDictEncode testDistRDFDictEncode.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../DistComputation.o ../StringDistributor.o ../../sys/endian.o
	$(CC) $(CFLAGS) -DTESTFILE='"foaf_1.nt"' -DNUMLINES=1 -o testDistRDFDictEncode testDistRDFDictEncode.cpp ../MPIDistPtrFileOutputStream.o ../MPIPacketDistributor.o ../MPIDelimFileInputStream.o ../../io/IOException.o ../../io/InputStream.o ../MPIFileInputStream.o ../../ptr/BadAllocException.o ../../ptr/Ptr.o ../../ptr/alloc.o ../../ex/TraceableException.o ../DistException.o ../../rdf/RDFTriple.o ../../rdf/RDFTerm.o ../../ucs/InvalidEncodingException.o ../../ptr/SizeUnknownException.o ../../lang/LangTag.o ../../iri/IRIRef.o ../../ucs/UTF8Iter.o ../../ucs/nf.o ../../iri/MalformedIRIRefException.o ../../ucs/InvalidCodepointException.o ../../ucs/utf.o ../../lang/MalformedLangTagException.o ../MPIFileOutputStream.o ../../io/OutputStream.o ../../rdf/NTriplesReader.o ../DistComputation.o ../StringDistributor.o ../../sys/endian.o
	$(ECHO) [TEST] ./testDistRDFDictEncode `pwd`/foaf.nt `pwd`/foaf.out
	$(RUN) -np 4 ./testDistRDFDictEncode `pwd`/foaf.nt `pwd`/foaf.out
