include ../../Makefile.inc

SUBDIR	= sys/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testendian

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testendian : testendian.cpp ../endian.h ../endian.o ../ints.h ../sys.h
	$(ECHO) $(CC) $(CFLAGS) -o testendian testendian.cpp ../endian.o
	$(CC) $(CFLAGS) -o testendian testendian.cpp ../endian.o
	$(ECHO) [TEST] ./testendian
	./testendian
