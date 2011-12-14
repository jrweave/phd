include ../Makefile.inc

SUBDIR	= lang
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= MalformedLangTagException.o LangTag.o MalformedLangRangeException.o LangRange.o

all : build __tests__

thorough : runtests
	cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(OBJS)

prep : lang_arrays.h lang_arrays.cpp

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
	-$(RM) -vf lang_arrays.*
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

lang_arrays.h : lang_arrays.cpp

lang_arrays.cpp : scripts/index.pl scripts/language-subtag-registry
	$(ECHO) generating lang_arrays.h and lang_arrays.cpp
	cd scripts; perl index.pl 2> ../lang_arrays.h > ../lang_arrays.cpp

MalformedLangTagException.o : MalformedLangTagException.h MalformedLangTagException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MalformedLangTagException.o MalformedLangTagException.cpp
	$(CC) $(CFLAGS) -c -o MalformedLangTagException.o MalformedLangTagException.cpp

LangTag.o : LangTag.h LangTag-inl.h LangTag.cpp lang_arrays.h lang_arrays.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o LangTag.o LangTag.cpp
	$(CC) $(CFLAGS) -c -o LangTag.o LangTag.cpp

MalformedLangRangeException.o : MalformedLangRangeException.h MalformedLangRangeException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o MalformedLangRangeException.o MalformedLangRangeException.cpp
	$(CC) $(CFLAGS) -c -o MalformedLangRangeException.o MalformedLangRangeException.cpp

LangRange.o : LangRange.h LangRange-inl.h LangRange.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o LangRange.o LangRange.cpp
	$(CC) $(CFLAGS) -c -o LangRange.o LangRange.cpp
