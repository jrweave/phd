include ../Makefile.inc

SUBDIR	= ucs
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= InvalidCodepointException.o InvalidEncodingException.o nf.o utf.o UCSIter.o UTF8Iter.o UTF16Iter.o UTF32Iter.o

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

ucs_arrays.h : ucs_arrays.cpp

ucs_arrays.cpp : scripts/index.pl scripts/UnicodeData.txt scripts/DerivedNormalizationProps.txt scripts/HangulSyllableType.txt scripts/NormalizationTest.txt
	$(ECHO) generating ucs_arrays.h and ucs_arrays.cpp
	cd scripts; perl index.pl 2> ../ucs_arrays.cpp | tee ../ucs_arrays.h

InvalidCodepointException.o : InvalidCodepointException.h InvalidCodepointException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o InvalidCodepointException.o InvalidCodepointException.cpp
	$(CC) $(CFLAGS) -c -o InvalidCodepointException.o InvalidCodepointException.cpp

InvalidEncodingException.o : InvalidEncodingException.h InvalidEncodingException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o InvalidEncodingException.o InvalidEncodingException.cpp
	$(CC) $(CFLAGS) -c -o InvalidEncodingException.o InvalidEncodingException.cpp

nf.o : nf.h nf.cpp ucs_arrays.h ucs_arrays.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o nf.o nf.cpp
	$(CC) $(CFLAGS) -c -o nf.o nf.cpp

utf.o : utf.h utf-inl.h utf.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o utf.o utf.cpp
	$(CC) $(CFLAGS) -c -o utf.o utf.cpp

UCSIter.o : UCSIter.h UCSIter.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o UCSIter.o UCSIter.cpp
	$(CC) $(CFLAGS) -c -o UCSIter.o UCSIter.cpp

UTF8Iter.o : UTF8Iter.h UTF8Iter.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o UTF8Iter.o UTF8Iter.cpp
	$(CC) $(CFLAGS) -c -o UTF8Iter.o UTF8Iter.cpp

UTF16Iter.o : UTF16Iter.h UTF16Iter.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o UTF16Iter.o UTF16Iter.cpp
	$(CC) $(CFLAGS) -c -o UTF16Iter.o UTF16Iter.cpp

UTF32Iter.o : UTF32Iter.h UTF32Iter.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o UTF32Iter.o UTF32Iter.cpp
	$(CC) $(CFLAGS) -c -o UTF32Iter.o UTF32Iter.cpp
