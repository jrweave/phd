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
