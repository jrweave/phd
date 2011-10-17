include Makefile.inc

# When adding a new subdirectory, make sure to modify DIRS
DIRS      = ex
EXE       = main
OBJS      =
OBJLIBS		=
LIBS      = -L.

all : $(EXE)

runtests : all force_look
	$(ECHO) running tests
	-for d in $(DIRS); do (cd $$d; $(MAKE) runtests); done

clean :
	$(ECHO) cleaning up in .
	$(ECHO) -$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

# add main.o to dependencies
$(EXE) : $(OBJLIBS)
	$(ECHO) This is where compiling main would happen
#$(ECHO) $(LD) -o $(EXE) $(OBJS) $(LIBS)
#$(LD) -o $(EXE) $(OBJS) $(LIBS)

ex : force_look
	$(ECHO) looking into ex : $(MAKE) $(MFLAGS)
	cd ex; $(MAKE) $(MFLAGS)

force_look :
	true
