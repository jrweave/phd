#include "test/unit.h"
#include "rdf/NTriplesWriter.h"

#include <map>
#include "io/IFStream.h"
#include "io/OFStream.h"
#include "ptr/DPtr.h"
#include "rdf/NTriplesReader.h"
#include "rdf/RDFTriple.h"

using namespace io;
using namespace rdf;
using namespace std;

bool test1() {
  InputStream *is;
  NEW(is, IFStream, "foaf.nt");
  size_t count = 0;
  size_t nerrs = 0;
  DPtr<uint8_t> *bytes = is->read();
  while (bytes != NULL) {
    count += bytes->size();
    bytes->drop();
    bytes = is->read();
  }
  is->close();
  DELETE(is);
  PROG(count == 11137);

  NEW(is, IFStream, "foaf.nt");
  NTriplesReader *nt;
  NEW(nt, NTriplesReader, is);

  OutputStream *os;
  NEW(os, OFStream, "foaf.out");
  NTriplesWriter *ntw;
  NEW(ntw, NTriplesWriter, os);

  RDFTriple triple;
  count = 0;
  bool r;
  do {
    try {
      r = nt->read(triple);
      if (r) {
        ++count;
        ntw->write(triple);
      }
    } catch (TraceableException &e) {
      cerr << e.what() << endl;
      ++nerrs;
    }
  } while(r);
  PROG(nerrs == 0);
  PROG(count == 94);
  nt->close();
  ntw->close();
  DELETE(nt);
  DELETE(ntw);

  map<RDFTerm, RDFTerm, bool(*)(const RDFTerm&, const RDFTerm&)> bnodes(RDFTerm::cmplt0);
  NEW(is, IFStream, "foaf.nt");
  NEW(nt, NTriplesReader, is);
  NEW(is, IFStream, "foaf.out");
  NTriplesReader *ntr;
  NEW(ntr, NTriplesReader, is);
  while (nt->read(triple)) {
    RDFTriple triple2;
    PROG(ntr->read(triple2));
    RDFTerm terms1[3];
    RDFTerm terms2[3];
    terms1[0] = triple.getSubj();
    terms1[1] = triple.getPred();
    terms1[2] = triple.getObj();
    terms2[0] = triple2.getSubj();
    terms2[1] = triple2.getPred();
    terms2[2] = triple2.getObj();
    int i;
    for (i = 0; i < 3; ++i) {
      if (terms1[i].getType() != BNODE) {
        PROG(terms1[i].equals(terms2[i]));
      } else {
        PROG(terms2[i].getType() == BNODE);
        map<RDFTerm, RDFTerm, bool(*)(const RDFTerm&, const RDFTerm&)>::iterator it = bnodes.find(terms1[i]);
        if (it == bnodes.end()) {
          bnodes[terms1[i]] = terms2[i];
        } else {
          PROG(terms2[i].equals(it->second));
        }
      }
    }
  }
  PROG(!ntr->read(triple));
  DELETE(nt);
  DELETE(ntr);

  PASS;
}

int main(int argc, char **argv) {
  INIT;

  TEST(test1);

  FINAL;
}
