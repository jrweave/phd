include ../../Makefile.inc

SUBDIR	= ucs/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testnf testutf

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testutf : testutf.cpp ../utf.h ../utf.o ../UTF8Iter.o ../UTF16Iter.o ../UTF32Iter.o ../UCSIter.o
	$(ECHO) running test $(SUBDIR)/testutf
	$(ECHO) $(CC) $(CFLAGS) -o testutf testutf.cpp ../utf.o ../../sys/endian.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o ../UTF8Iter.o ../UTF16Iter.o ../UTF32Iter.o ../UCSIter.o
	$(CC) $(CFLAGS) -o testutf testutf.cpp ../utf.o ../../sys/endian.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o ../UTF8Iter.o ../UTF16Iter.o ../UTF32Iter.o ../UCSIter.o
	$(ECHO) [TEST] ./testutf
	./testutf

testnf : testnf.cpp ../nf.h ../nf.o
	$(ECHO) running test $(SUBDIR)/testnf
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running test $(SUBDIR)/testnf with -DUCS_NO_K
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_K -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -DUCS_NO_K -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running test $(SUBDIR)/testnf with -DUCS_NO_C
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_C -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_C -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_C -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -DUCS_NO_C -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running test $(SUBDIR)/testnf with -DUCS_NO_K -DUCS_NO_C
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -DUCS_NO_C -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -DUCS_NO_C -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_K -DUCS_NO_C -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -DUCS_NO_K -DUCS_NO_C -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running test $(SUBDIR)/testnf with -DUCS_TRUST_CODEPOINTS
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_TRUST_CODEPOINTS -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_TRUST_CODEPOINTS -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_TRUST_CODEPOINTS -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -DUCS_TRUST_CODEPOINTS -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running test $(SUBDIR)/testnf with -DUCS_PLAY_DUMB
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_PLAY_DUMB -c -o __nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_PLAY_DUMB -c -o __nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_PLAY_DUMB -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -DUCS_PLAY_DUMB -o testnf testnf.cpp ../__nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	-$(RM) -vf ../__nf.o
	$(ECHO) $(CC) $(CFLAGS) -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
	$(CC) $(CFLAGS) -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o ../InvalidEncodingException.o ../InvalidCodepointException.o
