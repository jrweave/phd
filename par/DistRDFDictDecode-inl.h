/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#include "par/DistRDFDictDecode.h"

#if DIST_RDF_DICT_DECODE_DEBUG
#include <iomanip>
#endif
#include "sys/endian.h"
#include "util/funcs.h"
#include "util/hash.h"

namespace par {

template<size_t N, typename ID, typename ENC>
DistRDFDictDecode<N, ID, ENC>::DistRDFDictDecode(const int rank,
    const int nproc, RDFWriter *writer, Distributor *dist, InputStream *in,
    RDFDictionary<ID, ENC> *dict, const bool discard_atypical)
    throw(BaseException<void*>, BadAllocException)
    : DistComputation(dist), dict(dict), gotten(false), count(0),
      nproc(nproc), rank(rank), discard_atypical(discard_atypical),
      curpos(3), writer(writer), input(in), nprocdone(0), ndonesent(0),
      current(NULL), ndonerecv(0) {
  if (writer == NULL) {
    THROW(BaseException<void*>, NULL, "RDFWriter *writer must not be NULL.");
  }
  if (in == NULL) {
    THROW(BaseException<void*>, NULL, "InputStream *in must not be NULL.");
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictDecode<N, ID, ENC>::DistRDFDictDecode(const int rank,
    const int nproc, RDFWriter *writer, Distributor *dist, InputStream *in,
    ENC &enc, RDFDictionary<ID, ENC> *dict, const bool discard_atypical)
    throw(BaseException<void*>, BadAllocException)
    : DistComputation(dist, enc), dict(dict), gotten(false), count(0),
      nproc(nproc), rank(rank), discard_atypical(discard_atypical),
      curpos(3), writer(writer), input(in), nprocdone(0), ndonesent(0),
      current(NULL), ndonerecv(0) {
  if (writer == NULL) {
    THROW(BaseException<void*>, NULL, "RDFWriter *writer must not be NULL.");
  }
  if (in == NULL) {
    THROW(BaseException<void*>, NULL, "InputStream *in must not be NULL.");
  }
}

template<size_t N, typename ID, typename ENC>
DistRDFDictDecode<N, ID, ENC>::~DistRDFDictDecode() throw(DistException) {
  if (!this->gotten) {
    DELETE(this->dict);
  }
  DELETE(this->writer);
  DELETE(this->input);
  if (this->current != NULL) {
    this->current->drop();
  }
}

template<size_t N, typename ID, typename ENC>
inline
DistRDFDictionary<N, ID, ENC> *DistRDFDictDecode<N, ID, ENC>::getDictionary()
    throw() {
  this->gotten = true;
  return this->dict;
}

template<size_t N, typename ID, typename ENC>
void DistRDFDictDecode<N, ID, ENC>::start() throw(TraceableException) {
  // do nothing
}

template<size_t N, typename ID, typename ENC>
int DistRDFDictDecode<N, ID, ENC>::pickup(DPtr<uint8_t> *&buffer, size_t &len)
    THROWS(BadAllocException, TraceableException) {
  if (!this->pending_responses.empty()) {
    const pending_response resp = this->pending_responses.front();
    DPtr<uint8_t> *p = resp.term.toUTF8String();
    len = p->size() + sizeof(uint32_t) + sizeof(int);
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }

#if DIST_RDF_DICT_DECODE_DEBUG
    _debugss << "send_to=" << resp.send_to << " n=" << resp.n << " astr=" << resp.term << " 4. RESPONDING " << this->rank << endl;
    cerr << _debugss.str();
    _debugss.str(string(""));
#endif

    uint8_t *write_to = buffer->dptr();
    int neg = -(this->rank) - 1;
    memcpy(write_to, &neg, sizeof(int));
    write_to += sizeof(int);
    memcpy(write_to, &resp.n, sizeof(uint32_t));
    write_to += sizeof(uint32_t);
    memcpy(write_to, p->dptr(), p->size());
    p->drop();
    this->pending_responses.pop_front();
#if DIST_RDF_DICT_DECODE_DEBUG
    ++DEBUG_SENT;
#endif
    return resp.send_to;
  }
  if (this->curpos > 2) {
    if (this->current != NULL) {
      this->current->drop();
    }
    this->current = this->input->read(3*N*sizeof(uint8_t));
    if (this->current == NULL) {
      if (this->ndonesent < this->nproc) {
        len = 1;
#if DIST_RDF_DICT_DECODE_DEBUG
        ++DEBUG_SENT;
#endif
        return this->ndonesent++;
      }
      if (!this->pending_positions.empty()) {
        return -1;
      }
      if (this->ndonerecv < this->nproc) {
        len = 1;
#if DIST_RDF_DICT_DECODE_DEBUG
        ++DEBUG_SENT;
#endif
        return this->ndonerecv++;
      }
      return this->nprocdone >= (this->nproc << 1) ? -2 : -1;
    }
#if DIST_RDF_DICT_DECODE_DEBUG
    ++DEBUG_READ;
#endif
    this->curpos = 0;
    pending_triple pend;
    pend.need = 3;
    this->pending_triples.push_back(pend);
    this->curpend = this->pending_triples.end();
    --this->curpend; 
  }
  ID id;
  memcpy(id.ptr(), this->current->dptr() + N*this->curpos, N*sizeof(uint8_t));
  typename Term2IMap::const_iterator t2i_iter = this->pending_term2i.find(id);
  if (t2i_iter != this->pending_term2i.end()) {
    pending_position pend;
    pend.triple = this->curpend;
    pend.pos = this->curpos;
    this->pending_positions.insert(
        pair<uint32_t, pending_position>(t2i_iter->second, pend));
    ++this->curpos;
    return -1;
  }
  RDFTerm term;
  if (this->dict->lookup(id, term)) {
    this->curpend->parts[this->curpos] = term;
    ++this->curpos;
    --this->curpend->need;
    if (this->curpend->need <= 0) {
      if (this->curpend->parts[0].isLiteral() ||
          this->curpend->parts[1].getType() != IRI) {
        if (this->discard_atypical) {
          this->pending_triples.erase(this->curpend);
          return -1;
        }
      }
      try {
        RDFTriple trip(this->curpend->parts[0], this->curpend->parts[1],
                       this->curpend->parts[2]);
        this->pending_triples.erase(this->curpend);
#if DIST_RDF_DICT_DECODE_DEBUG
        ++DEBUG_WRIT;
#endif
        this->writer->write(trip);
      } JUST_RETHROW(TraceableException, "(rethrow)")
    }
    return -1;
  }
  uint32_t top;
  memcpy(&top, id.ptr(), sizeof(uint32_t));
  if (is_little_endian()) {
    reverse_bytes(top);
  }
  int send_to = (int) top;
  len = ID::size() + sizeof(uint32_t) + sizeof(int);
  if (buffer->size() < len) {
    buffer->drop();
    try {
      NEW(buffer, MPtr<uint8_t>, len);
    } RETHROW_BAD_ALLOC
  }

#if DIST_RDF_DICT_DECODE_DEBUG
  _debugss << "send_to=" << send_to << " n=" << this->count << " id=" << hex;
  _debugss << setfill('0');
  const uint8_t *b = id.ptr();
  const uint8_t *e = b + ID::size();
  for (; b != e; ++b) {
    _debugss << setw(2) << (const int)*b << ":";
  }
  _debugss << dec << " 1. REQUESTING LOOKUP " << this->rank << endl;
  cerr << _debugss.str();
  _debugss.str(string(""));
#endif

  pending_position pend;
  pend.triple = this->curpend;
  pend.pos = this->curpos;
  this->pending_positions.insert(
      pair<uint32_t, pending_position>(this->count, pend));

  this->pending_term2i.insert(pair<ID, uint32_t>(id, this->count));
  this->pending_i2term.insert(pair<uint32_t, ID>(this->count, id));
  uint8_t *write_to = buffer->dptr();
  memcpy(write_to, &this->rank, sizeof(int));
  write_to += sizeof(int);
  memcpy(write_to, &this->count, sizeof(uint32_t));
  ++this->count;
  write_to += sizeof(uint32_t);
  memcpy(write_to, id.ptr(), ID::size());
  ++this->curpos;
#if DIST_RDF_DICT_DECODE_DEBUG
  ++DEBUG_SENT;
#endif
  return send_to;
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID, typename ENC>
void DistRDFDictDecode<N, ID, ENC>::dropoff(DPtr<uint8_t> *msg)
    THROWS(TraceableException) {
#if DIST_RDF_DICT_DECODE_DEBUG
  ++DEBUG_RECV;
#endif
  if (msg->size() <= 1) {
    ++this->nprocdone;
    return;
  }
  pending_response resp;
  const uint8_t *read_from = msg->dptr();
  memcpy(&resp.send_to, read_from, sizeof(int));
  read_from += sizeof(int);
  memcpy(&resp.n, read_from, sizeof(uint32_t));
  // non-negative "send_to" means this is a lookup request
  if (resp.send_to >= 0) {
    read_from += sizeof(uint32_t);
    ID id;
    memcpy(id.ptr(), read_from, ID::size());

#if DIST_RDF_DICT_DECODE_DEBUG
    _debugss << "send_to=" << resp.send_to << " n=" << resp.n << " id=" << hex;
    _debugss << setfill('0');
    const uint8_t *b = id.ptr();
    const uint8_t *e = b + ID::size();
    for (; b != e; ++b) {
      _debugss << setw(2) << (const int)*b << ":";
    }
    _debugss << dec << " 2. RECEIVED LOOKUP REQUEST " << this->rank << endl;
    cerr << _debugss.str();
    _debugss.str(string(""));
#endif

    resp.term = this->dict->decode(id);
    this->pending_responses.push_back(resp);

#if DIST_RDF_DICT_DECODE_DEBUG
    _debugss << "send_to=" << resp.send_to << " n=" << resp.n << " astr=" << resp.term << " 3. PENDING RESPONSE " << this->rank << endl;
    cerr << _debugss.str();
    _debugss.str(string(""));
#endif

    return;
  }
  // otherwise, negative "send_to" means it is a response
  DPtr<uint8_t> *termstr = msg->sub(sizeof(int) + sizeof(uint32_t),
      msg->size() - sizeof(int) - sizeof(uint32_t));
  resp.term = RDFTerm::parse(termstr);
  termstr->drop();

#if DIST_RDF_DICT_DECODE_DEBUG
  _debugss << "send_to=" << this->rank << " n=" << resp.n << " astr=" << resp.term << " 5. RECEIVED RESPONSE " << this->rank << endl;
  cerr << _debugss.str();
  _debugss.str(string(""));
#endif

  pair<typename multimap<uint32_t, pending_position>::iterator,
       typename multimap<uint32_t, pending_position>::iterator> range =
          this->pending_positions.equal_range(resp.n);
  typename multimap<uint32_t, pending_position>::iterator it;
  for (it = range.first; it != range.second; ++it) {
    it->second.triple->parts[it->second.pos] = resp.term;
    if (--it->second.triple->need <= 0) {
      if (it->second.triple->parts[0].isLiteral() ||
          it->second.triple->parts[1].getType() != IRI) {
        if (this->discard_atypical) {
          this->pending_triples.erase(it->second.triple);
          continue;
        }
      }
      try {
        RDFTriple trip(it->second.triple->parts[0],
                       it->second.triple->parts[1],
                       it->second.triple->parts[2]);
        this->pending_triples.erase(it->second.triple);
#if DIST_RDF_DICT_DECODE_DEBUG
        ++DEBUG_WRIT;
#endif
        this->writer->write(trip);
      } JUST_RETHROW(TraceableException, "(rethrow)")

    }
  }
  this->pending_positions.erase(range.first, range.second);
  typename I2TermMap::iterator i2t = this->pending_i2term.find(resp.n);
  this->dict->force(i2t->second, resp.term);
  this->pending_term2i.erase(i2t->second);
  this->pending_i2term.erase(i2t);
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID, typename ENC>
void DistRDFDictDecode<N, ID, ENC>::finish() THROWS(TraceableException) {
#if DIST_RDF_DICT_ENCODE_DEBUG
  cout << "Pending Responses " << this->pending_responses.size() << endl;
  cout << "Pending Triples " << this->pending_triples.size() << endl;
  cout << "Pending Positions " << this->pending_positions.size() << endl;
  cout << "Pending Term2I " << this->pending_term2i.size() << endl;
  cout << "Pending I2Term " << this->pending_i2term.size() << endl;
  cout << "READ " << DEBUG_READ << endl;
  cout << "SENT " << DEBUG_SENT << endl;
  cout << "RECV " << DEBUG_RECV << endl;
  cout << "WRIT " << DEBUG_WRIT << endl;
#endif
  this->writer->close();
  this->input->close();
  // Sanity check
  if (!this->pending_responses.empty() || !this->pending_triples.empty() ||
      !this->pending_positions.empty() || !this->pending_term2i.empty() ||
      !this->pending_i2term.empty()) {
    THROW(TraceableException,
          "Oh no!  Stopped decoding, but there's more work to be done!");
  }
}
TRACE(TraceableException, "(trace)")

template<size_t N, typename ID, typename ENC>
void DistRDFDictDecode<N, ID, ENC>::fail() throw() {
  try {
    if (this->writer != NULL) {
      this->writer->close();
    }
    if (this->input != NULL) {
      this->input->close();
    }
  } catch (TraceableException &e) {
    // Give up cleaning up.
  }
}

}
