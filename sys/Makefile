include ../Makefile.inc

SUBDIR	= sys
CFLAGS  = $(PRJCFLAGS) -I..
OBJS		= endian.o char.o

all : build __tests__

thorough : runtests
	cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(OBJS)

prep :

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
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

endian.o : endian.cpp endian-inl.h endian.h ints.h
	$(ECHO) $(CC) $(CFLAGS) -c -o endian.o endian.cpp
	$(CC) $(CFLAGS) -c -o endian.o endian.cpp

char.o : char.cpp char.h ints.h
	$(ECHO) $(CC) $(CFLAGS) -c -o char.o char.cpp
	$(CC) $(CFLAGS) -c -o char.o char.cpp
