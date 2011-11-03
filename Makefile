include Makefile.inc

# When adding a new subdirectory, make sure to modify DIRS
DIRS      = test sys util ex ptr ucs
EXE       = main
OBJS      =
OBJLIBS		=
LIBS      = -L.

all : $(DIRS) $(EXE)

runtests : all force_look
	$(ECHO) running tests
	-for d in $(DIRS); do (cd $$d; $(MAKE) runtests); done

clean :
	$(ECHO) cleaning up in .
	-$(RM) -vf $(EXE) $(OBJS) $(OBJLIBS)
	-$(RM) -vfr *.dSYM
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

force_look :
	true

# add main.o to dependencies
$(EXE) : $(OBJLIBS)
	$(ECHO) This is where compiling main would happen
#$(ECHO) $(LD) -o $(EXE) $(OBJS) $(LIBS)
#$(LD) -o $(EXE) $(OBJS) $(LIBS)

test : force_look
	$(ECHO) looking into test : $(MAKE) $(MFLAGS)
	cd test; $(MAKE) $(MFLAGS)

sys : test force_look
	$(ECHO) looking into sys : $(MAKE) $(MFLAGS)
	cd sys; $(MAKE) $(MFLAGS)

util : test force_look
	$(ECHO) looking into util : $(MAKE) $(MFLAGS)
	cd util; $(MAKE) $(MFLAGS)

ex : sys force_look
	$(ECHO) looking into ex : $(MAKE) $(MFLAGS)
	cd ex; $(MAKE) $(MFLAGS)

ptr : ex force_look
	$(ECHO) looking into ptr : $(MAKE) $(MFLAGS)
	cd ptr; $(MAKE) $(MFLAGS)

ucs : ptr force_look
	$(ECHO) looking into ucs : $(MAKE) $(MFLAGS)
	cd ucs; $(MAKE) $(MFLAGS)
