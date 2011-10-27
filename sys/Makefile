include ../Makefile.inc

SUBDIR	= sys
CFLAGS  = $(PRJCFLAGS) -I..
OBJS		=

# no __tests__
all : $(OBJS)

__tests__ : force_look
	$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	cd __tests__; $(MAKE) $(MFLAGS)

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
