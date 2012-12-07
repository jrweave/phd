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

SUBDIR	= 3rd
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
INSTALLS =
OBJS		=
ifeq ($(USE_3RD_LZO), yes)
INSTALLS += lzo
OBJS		+= lzo/src/lzo1x_1.o lzo/src/lzo_util.o lzo/src/lzo_init.o lzo/src/lzo1x_d2.o
endif

#add __tests__
all : build

thorough : runtests
	#cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(INSTALLS) $(OBJS)

prep :

__tests__ : force_look
	$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	#cd __tests__; $(MAKE) $(MFLAGS)

#had dependency all
runtests :
	#$(ECHO) running tests for $(SUBDIR) : $(MAKE) $(MFLAGS) runtests
	#cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	-$(RM) -vf $(OBJS)
	-$(RM) -vfr *.tar *.gz *.bz2
	-$(RM) -vfr *.dSYM
	#cd __tests__; $(MAKE) $(MFLAGS) clean

distclean : clean
	-$(RM) -vfr $(INSTALLS)

force_look :
	true

lzo : setup-lzo.sh
	$(ECHO) setting up LZO
	./setup-lzo.sh

lzo/src/lzo1x_1.o : lzo/src/lzo1x_1.c
	$(ECHO) $(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo1x_1.o lzo/src/lzo1x_1.c
	$(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo1x_1.o lzo/src/lzo1x_1.c

lzo/src/lzo_util.o : lzo/src/lzo_util.c
	$(ECHO) $(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo_util.o lzo/src/lzo_util.c
	$(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo_util.o lzo/src/lzo_util.c

lzo/src/lzo_init.o : lzo/src/lzo_init.c
	$(ECHO) $(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo_init.o lzo/src/lzo_init.c
	$(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo_init.o lzo/src/lzo_init.c

lzo/src/lzo1x_d2.o : lzo/src/lzo1x_d2.c
	$(ECHO) $(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo1x_d2.o lzo/src/lzo1x_d2.c
	$(CC) $(CFLAGS) -Ilzo/include -c -o lzo/src/lzo1x_d2.o lzo/src/lzo1x_d2.c
