#include "iri/IRIRef.h"
#include "ptr/MPtr.h"
#include "rdf/RDFTerm.h"
#include "rdf/RDFTriple.h"
#include "ucs/InvalidCodepointException.h"
#include "ucs/InvalidEncodingException.h"

using namespace iri;
using namespace ptr;
using namespace rdf;
using namespace std;
using namespace ucs;

DPtr<uint8_t> *s2p(const char *str) {
  DPtr<uint8_t> *p;
  NEW(p, MPtr<uint8_t>, strlen(str));
  ascii_strcpy(p->dptr(), str);
  return p;
}

void print(DPtr<uint8_t> *p) {
  const uint8_t *mark = p->dptr();
  const uint8_t *end = mark + p->size();
  for (; mark != end; ++mark) {
    cout << to_lchar(*mark);
  }
  cout << endl;
}

int main (int argc, char **argv) {
  DPtr<uint8_t> *p = s2p("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
  IRIRef iri(p);
  p->drop();
  p = iri.getPart(FRAGMENT);
  print(p);
  p->drop();
  iri.resolve(NULL);
  p = iri.getPart(FRAGMENT);
  print(p);
  p->drop();
  p = iri.getUTF8String();
  print(p);
  p->drop();

  p = s2p("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>");
  RDFTerm term = RDFTerm::parse(p);
  p->drop();
  p = term.toUTF8String();
  print(p);
  p->drop();

  RDFTriple triple(term, term, term);
  p = triple.toUTF8String(true);
  print(p);
  p->drop();

  p = s2p("file:///path");
  iri = IRIRef(p);
  cerr << iri << endl;
  p->drop();

  cerr << BaseException<IRIRef>(__FILE__, __LINE__, iri).what() << endl;
  try {
    term = RDFTerm::parse(s2p("</relative/iri>"));
  } catch (BaseException<IRIRef> &e) {
    cerr << e.what() << endl;
  }

  cerr << InvalidCodepointException(__FILE__, __LINE__, UINT32_C(0)).what() << endl;
  cerr << InvalidEncodingException(__FILE__, __LINE__, "Bad UTF.", UINT32_C(0)).what() << endl;
}
