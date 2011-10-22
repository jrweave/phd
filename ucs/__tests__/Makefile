include ../../Makefile.inc

SUBDIR	= ucs/__tests__
CFLAGS	= $(PRJCFLAGS) -I../..
TESTS		= testucs_lookup

all :

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	$(ECHO) -$(RM) -f $(TESTS)
	-$(RM) -vf $(TESTS)
	-$(RM) -vfr *.dSYM

runtests : $(TESTS)

force_look :
	true

testucs_lookup : force_look testucs_lookup.cpp ../ucs_lookup.h ../ucs_arrays.h ../ucs_lookup.cpp
	$(ECHO) $(CC) $(CFLAGS) -o testucs_lookup testucs_lookup.cpp ../ucs_lookup.o
	$(CC) $(CFLAGS) -o testucs_lookup testucs_lookup.cpp ../ucs_lookup.o
	$(ECHO) [TEST] ./testucs_lookup
	./testucs_lookup
