#include "par/DistRDFDictEncode.h"

#include "util/hash.h"

namespace par {

template<size_t N, typename ID, typename ENC>
DistRDFDictionary<N, ID, ENC>::DistRDFDictionary(const int rank) throw()
    : RDFDictionary<ID, ENC>(), rank(rank) {
  if (N <= sizeof(int)) {
    THROW(TraceableException, "N must be > sizeof(int).");
  }
  memcpy(&this->counter, &rank, sizeof(int));
  if (rank == 0) {
    ++this->counter;
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictionary<N, ID, ENC>::DistRDFDictionary(
    const int rank, const ENC &enc) throw()
    : RDFDictionary<ID, ENC>(enc), rank(rank) {
  if (N <= sizeof(int)) {
    THROW(TraceableException, "N must be > sizeof(int).");
  }
  memcpy(&this->counter, &rank, sizeof(int));
  if (rank == 0) {
    ++this->counter;
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictionary<N, ID, ENC>::~DistRDFDictionary() throw() {
  // do nothing
}

template<size_t N, typename ID, typename ENC>
bool DistRDFDictionary<N, ID, ENC>::nextID(ID &id) {
  int proc;
  memcpy(&proc, &this->counter, sizeof(int));
  if (proc != this->rank) {
    return false;
  }
  id = this->counter;
  ++this->counter;
  return true;
}

template<size_t N, typename ID, typename ENC>
void DistRDFDictionary<N, ID, ENC>::set(const RDFTerm &term, const ID &id) {
  this->term2id[term] = id;
  this->id2term[id] = term;
}

template<size_t N, typename ID, typename ENC>
ID DistRDFDictionary<N, ID, ENC>::encode(const RDFTerm &term) {
  ID id;
  if (this->lookup(term, id)) {
    return id;
  }
  THROW(TraceableException,
        "Cannot encode term in distributed dictionary.");
}

template<size_t N, typename ID, typename ENC>
ID DistRDFDictionary<N, ID, ENC>::locallyEncode(const RDFTerm &term) {
  try {
    return RDFDictionary<ID, ENC>::encode(term);
  } JUST_RETHROW(TraceableException, "Couldn't locally encode term.")
}




// DistRDFDictEncode

template<size_t N, typename ID, typename ENC>
DistRDFDictEncode<N, ID, ENC>::DistRDFDictEncode(const int rank,
    const int nproc, RDFReader *reader, Distributor *dist, OutputStream *out)
    throw(BaseException<void*>, BadAllocException)
    : DistComputation(dist), dict(NULL), gotten(false), count(0),
      nproc(nproc), pending_term2i(Term2IMap(RDFTerm::cmplt0)),
      curpos(3), reader(reader), output(out), nprocdone(0), ndonesent(0) {
  try {
    NEW(dist, WHOLE(DistRDFDictionary<ID, ENC>), rank);
  } RETHROW_BAD_ALLOC
  try {
    NEW(this->outbuf, MPtr<uint8_t>, 3*N*sizeof(uint8_t));
  } RETHROW_BAD_ALLOC
  if (reader == NULL) {
    THROW(BaseException<void*>, NULL, "RDFReader *reader must not be NULL.");
  }
  if (out == NULL) {
    THROW(BaseException<void*>, NULL, "OutputStream *out must not be NULL.");
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictEncode<N, ID, ENC>::DistRDFDictEncode(const int rank,
    const int nproc, RDFReader *reader, Distributor *dist, OutputStream *out,
    ENC &enc)
    throw(BaseException<void*>, BadAllocException)
    : DistComputation(dist, enc), dict(NULL), gotten(false), count(0),
      nproc(nproc), pending_term2i(Term2IMap(RDFTerm::cmplt0)),
      curpos(3), reader(reader), output(out), nprocdone(0), ndonesent(0) {
  try {
    NEW(dist, WHOLE(DistRDFDictionary<ID, ENC>), rank);
  } RETHROW_BAD_ALLOC
  try {
    NEW(this->outbuf, MPtr<uint8_t>, 3*N*sizeof(uint8_t));
  } RETHROW_BAD_ALLOC
  if (reader == NULL) {
    THROW(BaseException<void*>, NULL, "RDFReader *reader must not be NULL.");
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictEncode<N, ID, ENC>::~DistRDFDictEncode() throw(DistException) {
  if (!this->gotten) {
    DELETE(this->dict);
  }
  this->outbuf->drop();
  DELETE(this->reader);
  DELETE(this->output);
}

template<size_t N, typename ID, typename ENC>
inline
DistRDFDictionary<ID, ENC> *DistRDFDictEncode<N, ID, ENC>::getDictionary()
    throw() {
  this->gotten = true;
  return this->dict;
}

template<size_t N, typename ID, typename ENC>
void DistRDFDictEncode<N, ID, ENC>::start() throw(TraceableException) {
  // do nothing
}

template<size_t N, typename ID, typename ENC>
int DistRDFDictEncode<N, ID, ENC>::pickup(DPtr<uint8_t> *&buffer, size_t &len)
    THROWS(BadAllocException, TraceableException) {
  if (!this->pending_responses.empty()) {
    len = (ID::size() << 3) + sizeof(uint32_t) + sizeof(int);
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    const pending_response resp = this->pending_responses.front();
    uint8_t *write_to = buffer->dptr();
    int neg = -resp.send_to - 1;
    memcpy(write_to, &neg, sizeof(int));
    write_to += sizeof(int);
    memcpy(write_to, &resp.n, sizeof(uint32_t));
    write_to += sizeof(uint32_t);
    memcpy(write_to, &resp.id, N);
    this->pending_responses.pop_front();
    return resp.send_to;
  }
  if (this->curpos > 2) {
    if (!this->reader->read(this->current)) {
      if (this->ndonesent < this->nproc) {
        len = 1;
        return this->ndonesent++;
      }
      return this->nprocdone >= this->nproc ? -2 : -1;
    }
    this->curpos = 0;
    pending_triple pend;
    pend.need = 3;
    this->pending_triples.push_back(pend);
    this->curpend = this->pending_triples.end();
    --this->curpend; 
  }
  RDFTerm term;
  switch (this->curpos) {
  case 0:
    term = this->current.getSubj();
    break;
  case 1:
    term = this->current.getPred();
    break;
  case 2:
    term = this->current.getObj();
    break;
  }
  Term2IMap::const_iterator t2i_iter = this->pending_term2i.find(term);
  if (t2i_iter != this->pending_term2i.end()) {
    pending_position pend;
    pend.triple = this->curpend;
    pend.pos = this->curpos;
    this->pending_positions.insert(
        pair<uint32_t, pending_position>(t2i_iter->second, pend));
    ++this->curpos;
    return -1;
  }
  ID id;
  if (this->dict->lookup(term, id)) {
    this->curpend->parts[this->curpos] = id;
    --this->curpend->need;
    if (this->curpend->need <= 0) {
      pending_triple pend = *this->curpend;
      this->pending_triples.erase(this->curpend);
      if (!this->outbuf->alone()) {
        this->outbuf = this->outbuf->stand();
      }
      uint8_t *write_to = this->outbuf->dptr();
      memcpy(write_to, &pend.parts[0], N*sizeof(uint8_t));
      write_to += N;
      memcpy(write_to, &pend.parts[1], N*sizeof(uint8_t));
      write_to += N;
      memcpy(write_to, &pend.parts[2], N*sizeof(uint8_t));
      this->output->write(this->outbuf);
    }
    ++this->curpos;
    return -1;
  }
  DPtr<uint8_t> *termstr = term.toUTF8String();
  int send_to = (int) hash_jenkins_one_at_a_time(termstr->dptr(),
                                                 termstr->dptr() + len)
                      % this->nproc;
  if (send_to == this->dict->rank) {
    this->dropoff(termstr);
    termstr->drop();
    ++this->curpos;
    return -1;
  }
  len = termstr->size() + sizeof(uint32_t) + sizeof(int);
  if (buffer->size() < len) {
    buffer->drop();
    try {
      NEW(buffer, MPtr<uint8_t>, len);
    } RETHROW_BAD_ALLOC
  }
  uint8_t *write_to = buffer->dptr();
  memcpy(write_to, &this->dict->rank, sizeof(int));
  write_to += sizeof(int);
  memcpy(write_to, &this->count, sizeof(uint32_t));
  ++this->count;
  write_to += sizeof(uint32_t);
  memcpy(write_to, termstr->dptr(), termstr->size());
  ++this->curpos;
  return send_to;
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID=RDFID<N>, typename ENC=RDFEncoder<ID> >
void DistRDFDictEncode<N, ID, ENC>::dropoff(DPtr<uint8_t> *msg)
    THROWS(TraceableException) {
  if (msg->size() <= 1) {
    ++this->nprocdone;
    return;
  }
  pending_response resp;
  const uint8_t *read_from = msg->dptr();
  memcpy(&resp.send_to, read_from, sizeof(int));
  read_from += sizeof(int);
  memcpy(&resp.n, read_from, sizeof(uint32_t));
  if (resp.send_to >= 0) {
    DPtr<uint8_t> *termstr = msg->sub(sizeof(int) + sizeof(uint32_t));
    termstr = termstr->stand();
    RDFTerm term = RDFTerm::parse(termstr);
    resp.id = this->dict->locallyEncode(term);
    this->pending_responses.push_back(resp);
    return;
  }
  read_from += sizeof(uint32_t);
  memcpy(&resp.id, read_from, N);
  pair<multimap<uint32_t, pending_position>::iterator,
       multimap<uint32_t, pending_position>::iterator> range =
          this->pending_positions.equal_range(resp.n);
  multimap<uint32_t, pending_position>::iterator it;
  for (it = range.first; it != range.second; ++it) {
    it->second->triple->parts[it->second->pos] = resp.id;
    if (--it->second->triple->need <= 0) {
      pending_triple pend_trip = it->second->triple;
      this->pending_triples.erase(it->second->triple);
      if (!this->outbuf->alone()) {
        this->outbuf = this->outbuf->stand();
      }
      uint8_t *write_to = this->outbuf->dptr();
      memcpy(write_to, &pend_trip.parts[0], N);
      write_to += N;
      memcpy(write_to, &pend_trip.parts[1], N);
      write_to += N;
      memcpy(write_to, &pend_trip.parts[2], N);
      this->output->write(this->outbuf);
    }
  }
  this->pending_positions.erase(range.first, range.second);
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID, typename ENC>
void DistRDFDictEncode<N, ID, ENC>::finish() THROWS(TraceableException) {
  this->reader->close();
  this->reader = NULL;
  this->output->close();
  this->output = NULL;
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID, typename ENC>
void DistRDFDictEncode<N, ID, ENC>::fail() throw() {
  try {
    if (this->reader != NULL) {
      this->reader->close();
    }
    if (this->output != NULL) {
      this->output->close();
    }
  } catch (TraceableException &e) {
    // Give up cleaning up.
  }
}

}
