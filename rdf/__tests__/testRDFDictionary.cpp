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

template<size_t N>
bool bitseq(uint64_t test, RDFID<N> id) {
  bool ret = true;
  cerr << "==== BEGIN ====" << endl;
  while (test != 0) {
    bool testbit = ((test & UINT64_C(1)) != 0);
    cerr << (testbit ? 1 : 0) << ":" << (id[0] ? 1 : 0) << endl;
    ret &= (!testbit ^ id[0]);
    test >>= 1;
    id >>= 1;
  }
  cerr << "==== END (final test at return) ====" << endl;
  return ret && id == RDFID<N>::zero();
}

bool testRDFID() {
  RDFID<7> id7 = RDFID<7>::zero();
  rdf8_t id8 = rdf8_t::zero();
  uint64_t i8 = UINT64_C(0);
  PROG(bitseq<7>(i8, id7));
  PROG(bitseq<8>(i8, id8));
  ++id7; ++id8; ++i8;
  PROG(bitseq<7>(i8, id7));
  PROG(bitseq<8>(i8, id8));
  id7 ^= ++++++++++RDFID<7>::zero();
  id8 ^= ++++++++++rdf8_t::zero();
  i8 ^= UINT64_C(5);
  PROG(bitseq<7>(i8, id7));
  PROG(bitseq<8>(i8, id8));
  // TODO include more testing here
  PASS;
}

template<size_t N>
bool testN() {
  RDFDictionary<RDFID<N> > dict;
  RDFID<N> id;

  RDFTerm *term = s2t("<tag:jrweave@gmail.com,2012:test>");
  PROG(dict(*term, id));
  RDFTerm term2;
  PROG(dict(id, term2));
  PROG(term->equals(term2));
  DELETE(term);

  RDFID<N> id2;
  term = s2t("_:blank");
  PROG(dict(*term, id2));
  PROG(id != id2);
  PROG(dict(id2, term2));
  PROG(term->equals(term2));
  PROG(dict(id, term2));
  PROG(!term->equals(term2));
  DELETE(term);

  RDFID<N> id3;
  PROG((id != id3 && id != id2) ^ dict(id3, term2));
  
  PASS;
}

bool test8() {
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

  TEST(test8);
  TEST(testN<12>);
  TEST(testRDFID);

  FINAL;
}
