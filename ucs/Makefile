include ../Makefile.inc

SUBDIR	= ucs
CFLAGS  = $(PRJCFLAGS) -I..
OBJS		= ucs_lookup.o

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
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

ucs_lookup.o : ucs_lookup.h  ucs_arrays.h ucs_lookup.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o ucs_lookup.o ucs_lookup.cpp
	$(CC) $(CFLAGS) -c -o ucs_lookup.o ucs_lookup.cpp
