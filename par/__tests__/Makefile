include ../../Makefile.inc

SUBDIR	= par/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testMPIDelimFileInputStream testMPIPacketDistributor

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
