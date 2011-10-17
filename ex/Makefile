include ../Makefile.inc

CFLAGS  = $(PRJCFLAGS) -I..
OBJS		=

all : $(OBJS) __tests__

__tests__ : force_look
	$(ECHO) looking into ex/__tests__ : $(MAKE) $(MFLAGS)
	cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	$(ECHO) running tests for ex : $(MAKE) $(MFLAGS) runtests
	cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in ex
	$(ECHO) -$(RM) -f $(OBJS)
	-$(RM) -f $(OBJS)
	cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true
