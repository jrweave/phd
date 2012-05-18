#include "rdf/RDFDictionary.h"

#include <iostream>

namespace rdf {

using namespace std;

template<typename ID, typename ENC>
RDFDictionary<ID, ENC>::RDFDictionary() throw()
    : term2id(Term2IDMap(RDFTerm::cmplt0)), counter(ID::zero()) {
  // do nothing
}

template<typename ID, typename ENC>
RDFDictionary<ID, ENC>::RDFDictionary(const ENC &enc) throw()
    : term2id(Term2IDMap(RDFTerm::cmplt0)), counter(ID::zero()),
      encoder(enc) {
  // do nothing
}

template<typename ID, typename ENC>
RDFDictionary<ID, ENC>::~RDFDictionary() throw() {
  // do nothing
}

template<typename ID, typename ENC>
bool RDFDictionary<ID, ENC>::operator()(const RDFTerm &term, ID &id) {
  if (this->encoder(term, id) && !id((ID::size() << 3) - 1, true)) {
    return true;
  }
  typename Term2IDMap::const_iterator it = this->term2id.find(term);
  if (it != this->term2id.end()) {
    id = it->second;
    return true;
  }
  if (this->counter[(ID::size() << 3) - 1]) {
    return false;
  }
  id = this->counter;
  this->term2id[term] = id;
  this->id2term[id] = term;
  ++this->counter;
  return true;
}

template<typename ID, typename ENC>
bool RDFDictionary<ID, ENC>::operator()(const ID &id, RDFTerm &term) {
  ID myid = id;
  if (myid((ID::size() << 3) - 1, false)) {
    return this->encoder(myid, term);
  }
  typename ID2TermMap::const_iterator it = this->id2term.find(myid);
  if (it != this->id2term.end()) {
    term = it->second;
    return true;
  }
  return false;
}

}
