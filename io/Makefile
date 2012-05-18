include ../Makefile.inc

SUBDIR	= io
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= IOException.o InputStream.o IFStream.o

# add __tests__
all : build

thorough : runtests
	#cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(OBJS)

prep : 

__tests__ : force_look
	$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	#cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	$(ECHO) running tests for $(SUBDIR) : $(MAKE) $(MFLAGS) runtests
	#cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	-$(RM) -vf $(OBJS) 
	-$(RM) -vfr *.dSYM
	#cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

IOException.o : IOException.h IOException.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o IOException.o IOException.cpp
	$(CC) $(CFLAGS) -c -o IOException.o IOException.cpp

InputStream.o : InputStream.h InputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o InputStream.o InputStream.cpp
	$(CC) $(CFLAGS) -c -o InputStream.o InputStream.cpp

IFStream.o : IFStream.h IFStream.cpp IStream.h IStream-inl.h
	$(ECHO) $(CC) $(CFLAGS) -c -o IFStream.o IFStream.cpp
	$(CC) $(CFLAGS) -c -o IFStream.o IFStream.cpp
