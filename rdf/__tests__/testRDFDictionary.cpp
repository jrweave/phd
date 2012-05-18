#include "test/unit.h"
#include "rdf/RDFDictionary.h"

#include "ptr/DPtr.h"
#include "rdf/RDFTerm.h"

using namespace ptr;
using namespace rdf;
using namespace std;

DPtr<uint8_t> *s2p(const char *cstr) {
  DPtr<uint8_t> *p;
  try {
    NEW(p, MPtr<uint8_t>, strlen(cstr));
  } RETHROW_BAD_ALLOC
  ascii_strcpy(p->dptr(), cstr);
  return p;
}

RDFTerm *p2t(DPtr<uint8_t> *p) {
  RDFTerm term = RDFTerm::parse(p);
  p->drop();
  RDFTerm *termp;
  NEW(termp, RDFTerm, term);
  return termp;
}

RDFTerm *s2t(const char *cstr) {
  return p2t(s2p(cstr));
}

bool test1() {
  RDFDictionary<> dict;
  rdf8_t id;
  RDFTerm *term = s2t("<tag:jrweave@gmail.com,2012:test>");
  PROG(dict(*term, id));
  RDFTerm term2;
  PROG(dict(id, term2));
  PROG(term->equals(term2));
  DELETE(term);

  rdf8_t id2;
  term = s2t("_:blank");
  PROG(dict(*term, id2));
  PROG(id != id2);
  PROG(dict(id2, term2));
  PROG(term->equals(term2));
  PROG(dict(id, term2));
  PROG(!term->equals(term2));
  DELETE(term);

  rdf8_t id3;
  PROG((id != id3 && id != id2) ^ dict(id3, term2));
  
  PASS;
}

int main(int argc, char **argv) {
  INIT;

  TEST(test1);

  FINAL;
}
