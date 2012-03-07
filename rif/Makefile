include ../Makefile.inc

SUBDIR	= rif
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= RIFConst.o RIFVar.o RIFTerm.o

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

RIFConst.o : RIFConst.h RIFConst-inl.h RIFConst.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o RIFConst.o RIFConst.cpp
	$(CC) $(CFLAGS) -c -o RIFConst.o RIFConst.cpp

RIFVar.o : RIFVar.h RIFVar-inl.h RIFVar.cpp RIFConst.o
	$(ECHO) $(CC) $(CFLAGS) -c -o RIFVar.o RIFVar.cpp
	$(CC) $(CFLAGS) -c -o RIFVar.o RIFVar.cpp

RIFTerm.o : RIFTerm.h RIFTerm-inl.h RIFTerm.cpp RIFConst.o RIFVar.o
	$(ECHO) $(CC) $(CFLAGS) -c -o RIFTerm.o RIFTerm.cpp
	$(CC) $(CFLAGS) -c -o RIFTerm.o RIFTerm.cpp
