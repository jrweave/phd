include ../Makefile.inc

SUBDIR	= ucs
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= nf.o utf.o

all : $(OBJS) __tests__

__tests__ : force_look
	$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	$(ECHO) running tests for $(SUBDIR) : $(MAKE) $(MFLAGS) runtests
	cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	-$(RM) -vf $(OBJS) 
	-$(RM) -vfr *.dSYM
	-$(RM) -vf ucs_arrays.*
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

ucs_arrays : scripts/index.pl scripts/UnicodeData.txt scripts/DerivedNormalizationProps.txt scripts/HangulSyllableType.txt scripts/NormalizationTest.txt
	$(ECHO) generating ucs_arrays.h and ucs_arrays.cpp
	cd scripts; perl index.pl 2> ../ucs_arrays.cpp | tee ../ucs_arrays.h

ucs_arrays.h : ucs_arrays.cpp

ucs_arrays.cpp : scripts/index.pl scripts/UnicodeData.txt scripts/DerivedNormalizationProps.txt scripts/HangulSyllableType.txt scripts/NormalizationTest.txt
	$(ECHO) generating ucs_arrays.h and ucs_arrays.cpp
	cd scripts; perl index.pl 2> ../ucs_arrays.cpp | tee ../ucs_arrays.h

nf.o : nf.h nf.cpp ucs_arrays.h ucs_arrays.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o nf.o nf.cpp
	$(CC) $(CFLAGS) -c -o nf.o nf.cpp

utf.o : utf.h utf.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o utf.o utf.cpp
	$(CC) $(CFLAGS) -c -o utf.o utf.cpp
