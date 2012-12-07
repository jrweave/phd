# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

include ../Makefile.inc

SUBDIR	= io
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= IOException.o InputStream.o OutputStream.o IStream.o OStream.o BufferedInputStream.o BufferedOutputStream.o
ifeq ($(USE_3RD_LZO), yes)
OBJS		+= LZOOutputStream.o LZOInputStream.o
endif


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

distclean : clean

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

IStream.o : IStream.h IStream-inl.h IStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o IStream.o IStream.cpp
	$(CC) $(CFLAGS) -c -o IStream.o IStream.cpp

OStream.o : OStream.h OStream-inl.h OStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o OStream.o OStream.cpp
	$(CC) $(CFLAGS) -c -o OStream.o OStream.cpp

BufferedInputStream.o : BufferedInputStream.h BufferedInputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o BufferedInputStream.o BufferedInputStream.cpp
	$(CC) $(CFLAGS) -c -o BufferedInputStream.o BufferedInputStream.cpp

BufferedOutputStream.o : BufferedOutputStream.h BufferedOutputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -c -o BufferedOutputStream.o BufferedOutputStream.cpp
	$(CC) $(CFLAGS) -c -o BufferedOutputStream.o BufferedOutputStream.cpp

LZOOutputStream.o : LZOOutputStream.h LZOOutputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -I../3rd/lzo/include -c -o LZOOutputStream.o LZOOutputStream.cpp
	$(CC) $(CFLAGS) -I../3rd/lzo/include -c -o LZOOutputStream.o LZOOutputStream.cpp

LZOInputStream.o : LZOInputStream.h LZOInputStream.cpp
	$(ECHO) $(CC) $(CFLAGS) -I../3rd/lzo/include -c -o LZOInputStream.o LZOInputStream.cpp
	$(CC) $(CFLAGS) -I../3rd/lzo/include -c -o LZOInputStream.o LZOInputStream.cpp
