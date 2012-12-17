# Copyright 2012 Jesse Weaver
# 
#    Licensed unlzo the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed unlzo the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations unlzo the License.

include ../Makefile.inc

SUBDIR	= main
CFLAGS  = $(PRJCFLAGS) -I.. -I/usr/include
OBJS		= normalize-nt prd2cnf refine refine-all refine_rules der encode-rules infer-rules
ifeq ($(USE_3RD_LZO), yes)
OBJS		+= lzo
ifeq ($(USE_PAR_MPI), yes)
OBJS		+= lzo-mpi red-mpi
endif
endif
ifeq ($(USE_PAR_MPI), yes)
OBJS		+= infer normalize-nt-mpi infer-rules-mpi
endif

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

distclean : clean
	-$(RM) -vf *.der *.lzo *.idx *.dct *.prd *.enc script/*.der scripts/*.lzo scripts/*.idx scripts/*.dct scripts/*.prd scripts/*.enc *.frc scripts/*.frc testfiles/*.der testfiles/*.lzo testfiles/*.idx testfiles/*.dct testsfiles/*.prd testfiles/*.enc testfiles/*.frc

force_look :
	true

normalize-nt : normalize-nt.cpp
	$(ECHO) $(CC) $(CFLAGS) -o normalize-nt normalize-nt.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o normalize-nt normalize-nt.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(LZO_3RD_OBJS)

normalize-nt-mpi : normalize-nt-mpi.cpp
	$(ECHO) $(CC) $(CFLAGS) -o normalize-nt-mpi normalize-nt-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o normalize-nt-mpi normalize-nt-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(LZO_3RD_OBJS)

infer : infer.cpp
	$(ECHO) $(CC) $(CFLAGS) -o infer infer.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o infer infer.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(LZO_3RD_OBJS)

prd2cnf : pattern-etc.cpp prd2cnf.cpp
	$(ECHO) $(CC) $(CFLAGS) -o prd2cnf prd2cnf.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o prd2cnf prd2cnf.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

refine : pattern-etc.cpp refine-etc.cpp refine.cpp
	$(ECHO) $(CC) $(CFLAGS) -o refine refine.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o refine refine.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

refine-all : pattern-etc.cpp refine-etc.cpp refine-all.cpp
	$(ECHO) $(CC) $(CFLAGS) -o refine-all refine-all.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o refine-all refine-all.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

refine_rules : refine_rules.cpp
	$(ECHO) $(CC) $(CFLAGS) -o refine_rules refine_rules.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o refine_rules refine_rules.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

lzo : lzo.cpp
	$(ECHO) $(CC) $(CFLAGS) -I../3rd/lzo/include -o lzo lzo.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -I../3rd/lzo/include -o lzo lzo.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

lzo-mpi : lzo-mpi.cpp
	$(ECHO) $(CC) $(CFLAGS) -I../3rd/lzo/include -o lzo-mpi lzo-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -I../3rd/lzo/include -o lzo-mpi lzo-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

der : der.cpp
	$(ECHO) $(CC) $(CFLAGS) -o der der.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o der der.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

red-mpi : red-mpi.cpp
	$(ECHO) $(CC) $(CFLAGS) -o red-mpi red-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o red-mpi red-mpi.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

encode-rules : encode-rules.cpp
	$(ECHO) $(CC) $(CFLAGS) -o encode-rules encode-rules.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)
	$(CC) $(CFLAGS) -o encode-rules encode-rules.cpp $(RDF_OBJS) $(EX_OBJS) $(UCS_OBJS) $(PTR_OBJS) $(LANG_OBJS) $(IO_OBJS) $(IRI_OBJS) $(PAR_OBJS) $(RIF_OBJS) $(LZO_3RD_OBJS)

infer-rules : infer-rules.cpp
	$(ECHO) $(CC) $(CFLAGS) -o infer-rules infer-rules.cpp
	$(CC) $(CFLAGS) -o infer-rules infer-rules.cpp

infer-rules-mpi : infer-rules-mpi.cpp
	$(ECHO) $(CC) $(CFLAGS) -o infer-rules-mpi infer-rules-mpi.cpp $(PTR_OBJS) $(EX_OBJS) $(IO_OBJS) $(LZO_3RD_OBJS) $(PAR_OBJS)
	$(CC) $(CFLAGS) -o infer-rules-mpi infer-rules-mpi.cpp $(PTR_OBJS) $(EX_OBJS) $(IO_OBJS) $(LZO_3RD_OBJS) $(PAR_OBJS)
