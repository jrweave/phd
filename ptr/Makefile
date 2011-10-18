include ../Makefile.inc

SUBDIR	= ptr
CFLAGS  = $(PRJCFLAGS) -I..
OBJS		= BadAllocException.o Ptr.o

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

BadAllocException.o : ../ex/TraceableException.h BadAllocException.h BadAllocException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o BadAllocException.o BadAllocException.cpp
	$(CC) $(CFLAGS) -c -o BadAllocException.o BadAllocException.cpp

Ptr.o : BadAllocException.o Ptr.h Ptr.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o Ptr.o Ptr.cpp
	$(CC) $(CFLAGS) -c -o Ptr.o Ptr.cpp
