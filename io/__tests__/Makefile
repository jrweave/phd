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

include ../../Makefile.inc

SUBDIR	= io/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testBufferedInputStream
ifeq ($(USE_3RD_LZO), yes)
TESTS		+= testLZOOutputStream testLZOInputStream
endif

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM
	-$(RM) -vf foaf.out

thorough : runtests

runtests : $(TESTS)

force_look :
	true

testBufferedInputStream : testBufferedInputStream.cpp ../BufferedInputStream.o
	$(ECHO) running test $(SUBDIR)/testBufferedInputStream
	$(ECHO) $(CC) $(CFLAGS) -o testBufferedInputStream testBufferedInputStream.cpp ../BufferedInputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o
	$(CC) $(CFLAGS) -o testBufferedInputStream testBufferedInputStream.cpp ../BufferedInputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o
	$(ECHO) [TEST] ./testBufferedInputStream
	./testBufferedInputStream

testLZOOutputStream : testLZOOutputStream.cpp ../LZOOutputStream.o ../BufferedOutputStream.o
	$(ECHO) running test $(SUBDIR)/testLZOOutputStream
	$(ECHO) $(CC) $(CFLAGS) -o testLZOOutputStream testLZOOutputStream.cpp ../LZOOutputStream.o ../BufferedOutputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o ../../3rd/lzo/src/lzo1x_1.o ../../ptr/SizeUnknownException.o ../OutputStream.o ../../3rd/lzo/src/lzo_util.o ../../3rd/lzo/src/lzo_init.o ../../3rd/lzo/src/lzo1x_d2.o
	$(CC) $(CFLAGS) -o testLZOOutputStream testLZOOutputStream.cpp ../LZOOutputStream.o ../BufferedOutputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o ../../3rd/lzo/src/lzo1x_1.o ../../ptr/SizeUnknownException.o ../OutputStream.o ../../3rd/lzo/src/lzo_util.o ../../3rd/lzo/src/lzo_init.o ../../3rd/lzo/src/lzo1x_d2.o
	$(ECHO) [TEST] ./testLZOOutputStream
	./testLZOOutputStream
	rm -fv *.enc

testLZOInputStream : testLZOInputStream.cpp ../LZOInputStream.o
	$(ECHO) running test $(SUBDIR)/testLZOInputStream
	$(ECHO) $(CC) $(CFLAGS) -o testLZOInputStream testLZOInputStream.cpp ../LZOInputStream.o ../BufferedOutputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o ../../3rd/lzo/src/lzo1x_1.o ../../ptr/SizeUnknownException.o ../OutputStream.o ../../3rd/lzo/src/lzo_util.o ../../3rd/lzo/src/lzo_init.o ../../3rd/lzo/src/lzo1x_d2.o
	$(CC) $(CFLAGS) -o testLZOInputStream testLZOInputStream.cpp ../LZOInputStream.o ../BufferedOutputStream.o ../InputStream.o ../../ptr/Ptr.o ../IOException.o ../../ptr/BadAllocException.o ../../ex/TraceableException.o ../../ptr/alloc.o ../../3rd/lzo/src/lzo1x_1.o ../../ptr/SizeUnknownException.o ../OutputStream.o ../../3rd/lzo/src/lzo_util.o ../../3rd/lzo/src/lzo_init.o ../../3rd/lzo/src/lzo1x_d2.o
	$(ECHO) [TEST] ./testLZOInputStream
	./testLZOInputStream
	rm -fv *.dec
