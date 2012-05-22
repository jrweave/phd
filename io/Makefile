include ../Makefile.inc

SUBDIR	= io
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= IOException.o InputStream.o OutputStream.o

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

InputStream.o : InputStream.h InputStream-inl.h InputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o InputStream.o InputStream.cpp
	$(CC) $(CFLAGS) -c -o InputStream.o InputStream.cpp

OutputStream.o : OutputStream.h OutputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o OutputStream.o OutputStream.cpp
	$(CC) $(CFLAGS) -c -o OutputStream.o OutputStream.cpp
