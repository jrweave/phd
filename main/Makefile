include ../Makefile.inc

SUBDIR	= main
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= normalize-nt testiri

all : build __tests__

thorough : runtests
	#cd __tests__; $(MAKE) $(MFLAGS) thorough

build : prep $(OBJS)

prep : 

__tests__ : force_look
	#$(ECHO) looking into $(SUBDIR)/__tests__ : $(MAKE) $(MFLAGS)
	#cd __tests__; $(MAKE) $(MFLAGS)

runtests : all
	#$(ECHO) running tests for $(SUBDIR) : $(MAKE) $(MFLAGS) runtests
	#cd __tests__; $(MAKE) $(MFLAGS) runtests

clean :
	$(ECHO) cleaning up in $(SUBDIR)
	-$(RM) -vf $(OBJS) 
	-$(RM) -vfr *.dSYM
	-$(RM) -vf rdf_constants.*
	#cd __tests__; $(MAKE) $(MFLAGS) clean

force_look :
	true

normalize-nt : normalize-nt.cpp
	$(ECHO) $(CC) $(CFLAGS) -o normalize-nt normalize-nt.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS)
	$(CC) $(CFLAGS) -o normalize-nt normalize-nt.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS)

testiri : testiri.cpp
	$(ECHO) $(CC) $(CFLAGS) -o testiri testiri.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS)
	$(CC) $(CFLAGS) -o testiri testiri.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS)
