CC        = mpicxx
PRJCFLAGS = -O3 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DSYSTEM=SYS_DEFAULT -DPTR_MEMDEBUG
LD        = g++
LDFLAGS   =
AR        = ar
ARFLAGS   =
RANLIB    = ranlib
RM        = rm
ECHO      = echo
RUN				= mpirun
TESTFILE	= /Users/jesseweaver/Desktop/_test_


EX_OBJS		= ../ex/TraceableException.o
IO_OBJS		= ../io/IOException.o ../io/InputStream.o ../io/OutputStream.o
IRI_OBJS		= ../iri/MalformedIRIRefException.o ../iri/IRIRef.o
LANG_OBJS		= ../lang/MalformedLangTagException.o ../lang/LangTag.o ../lang/MalformedLangRangeException.o ../lang/LangRange.o
PAR_OBJS		= ../par/MPIFileInputStream.o ../par/MPIDelimFileInputStream.o ../par/DistException.o ../par/MPIPacketDistributor.o ../par/StringDistributor.o ../par/MPIFileOutputStream.o ../par/MPIDistPtrFileOutputStream.o ../par/DistComputation.o
PTR_OBJS		= ../ptr/alloc.o ../ptr/BadAllocException.o ../ptr/Ptr.o ../ptr/SizeUnknownException.o
RDF_OBJS		= ../rdf/RDFTerm.o ../rdf/RDFTriple.o ../rdf/NTriplesReader.o ../rdf/NTriplesWriter.o
RIF_OBJS		= ../rif/RIFConst.o ../rif/RIFVar.o ../rif/RIFTerm.o ../rif/RIFAtomic.o ../rif/RIFCondition.o ../rif/RIFDictionary.o
SYS_OBJS		= ../sys/endian.o ../sys/char.o
UCS_OBJS		= ../ucs/InvalidCodepointException.o ../ucs/InvalidEncodingException.o ../ucs/nf.o ../ucs/utf.o ../ucs/UTF8Iter.o ../ucs/UTF16Iter.o ../ucs/UTF32Iter.o
UTIL_OBJS		=

SHELL     = /bin/sh

.SILENT :
