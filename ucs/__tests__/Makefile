include ../../Makefile.inc

SUBDIR	= ucs/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testnf

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testnf : force_look testnf.cpp ../nf.h ../nf.o
	$(ECHO) $(CC) $(CFLAGS) -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running tests for $(SUBDIR) with -DUCS_PLAY_DUMB
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_PLAY_DUMB -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_PLAY_DUMB -c -o nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_PLAY_DUMB -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -DUCS_PLAY_DUMB -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running tests for $(SUBDIR) with -DUCS_TRUST_CODEPOINTS
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_TRUST_CODEPOINTS -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_TRUST_CODEPOINTS -c -o nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_TRUST_CODEPOINTS -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -DUCS_TRUST_CODEPOINTS -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running tests for $(SUBDIR) with -DUCS_NO_K
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -c -o nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_K -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -DUCS_NO_K -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running tests for $(SUBDIR) with -DUCS_NO_C
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_C -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_C -c -o nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_C -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -DUCS_NO_C -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) running tests for $(SUBDIR) with -DUCS_NO_K -DUCS_NO_C
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -DUCS_NO_C -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -DUCS_NO_K -DUCS_NO_C -c -o nf.o nf.cpp; cd -
	$(ECHO) $(CC) $(CFLAGS) -DUCS_NO_K -DUCS_NO_C -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o. ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(CC) $(CFLAGS) -DUCS_NO_K -DUCS_NO_C -o testnf testnf.cpp ../nf.o ../../ptr/SizeUnknownException.o ../../ptr/Ptr.o ../../ex/TraceableException.o ../../ptr/BadAllocException.o
	$(ECHO) [TEST] ./testnf
	./testnf
	$(ECHO) $(CC) $(PRJCFLAGS) -I.. -c -o nf.o nf.cpp
	cd ..; $(CC) $(PRJCFLAGS) -I.. -c -o nf.o nf.cpp; cd -
