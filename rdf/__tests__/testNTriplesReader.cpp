#include "test/unit.h"
#include "rdf/NTriplesReader.h"

#include "io/IFStream.h"
#include "ptr/DPtr.h"
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
  PROG(count == 11141);

  NEW(is, IFStream, "foaf.nt");
  NTriplesReader *nt;
  NEW(nt, NTriplesReader, is);
  RDFTriple triple;
  count = 0;
  bool r;
  do {
    try {
      r = nt->read(triple);
      if (r) ++count;
    } catch (TraceableException &e) {
      cerr << e.what() << endl;
      ++nerrs;
    }
  } while(r);
  PROG(nerrs == 0);
  PROG(count == 94);
  nt->close();
  DELETE(nt);
  PASS;
}

int main(int argc, char **argv) {
  INIT;

  TEST(test1);

  FINAL;
}
