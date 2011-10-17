include ../../Makefile.inc

CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testBaseException

all : $(TESTS)

testBaseException : testBaseException.cpp ../BaseException.h ../BaseException-inl.h
	$(ECHO) $(LD) -o testBaseException testBaseException.cpp
	$(CC) $(CFLAGS) -o testBaseException testBaseException.cpp

clean :
	$(ECHO) cleaning up in ex/__tests__
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : all
	$(ECHO) ./testBaseException
	./testBaseException

force_look :
	true
