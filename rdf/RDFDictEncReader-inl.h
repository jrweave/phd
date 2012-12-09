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

#include "rdf/RDFDictEncReader.h"

#include "sys/endian.h"
#include "util/funcs.h"

namespace rdf {

using namespace sys;
using namespace util;

template<typename ID, typename ENC>
RDFDictEncReader<ID, ENC>::RDFDictEncReader(InputStream *is,
    RDFDictionary<ID, ENC> *dict, const bool discard_atypical,
    const bool own_dictionary)
    throw(BaseException<void*>, IOException)
    : input(is), dict(dict), buffer(NULL), offset(0),
      discard_atypical(discard_atypical),
      own_dictionary(own_dictionary) {
  if (this->input == NULL) {
    THROW(BaseException<void*>, NULL, "input must not be NULL.");
  }
  if (this->dict == NULL) {
    THROW(BaseException<void*>, NULL, "dict must not be NULL.");
  }
  try {
    this->buffer = this->input->read();
  } JUST_RETHROW(IOException, "Trouble doing first read in RDFDictEncReader.")
}

template<typename ID, typename ENC>
RDFDictEncReader<ID, ENC>::~RDFDictEncReader() THROWS(IOException) {
  DELETE(this->input);
  if (this->buffer != NULL) {
    this->buffer->drop();
  }
  if (this->own_dictionary) {
    DELETE(this->dict);
  }
}
TRACE(IOException, "Trouble deconstructing RDFDictEncReader.")

template<typename ID, typename ENC>
DPtr<uint8_t> *RDFDictEncReader<ID, ENC>::readNext(InputStream *is,
    const size_t len) {
  DPtr<uint8_t> *p = is->read(len);
  if (p == NULL || p->size() == len) {
    return p;
  }
  DPtr<uint8_t> *q;
  try {
    NEW(q, MPtr<uint8_t>, len);
  } RETHROW_BAD_ALLOC
  uint8_t *qp = q->dptr();
  const uint8_t *qend = qp + q->size();
  memcpy(qp, p->dptr(), p->size());
  qp += p->size();
  p->drop();
  while (qp != qend) {
    p = is->read(qend - qp);
    if (p == NULL) {
      q->drop();
      THROW(TraceableException, "Unexpected end of stream.");
    }
    memcpy(qp, p->dptr(), p->size());
    qp += p->size();
    p->drop();
  }
  return q;
}

template<typename ID, typename ENC>
RDFDictionary<ID, ENC> *RDFDictEncReader<ID, ENC>::readDictionary(InputStream *is,
    RDFDictionary<ID, ENC> *dict) {
  if (dict == NULL) {
    try {
      NEW(dict, WHOLE(RDFDictionary<ID, ENC>));
    } RETHROW_BAD_ALLOC
  }
  ID id;
  uint32_t len;
  DPtr<uint8_t> *p = RDFDictEncReader<ID, ENC>::readNext(is, ID::size() +
                                                         sizeof(uint32_t));
  while (p != NULL) {
    memcpy(id.ptr(), p->dptr(), ID::size());
    memcpy(&len, p->dptr() + ID::size(), sizeof(uint32_t));
    p->drop();
    if (is_little_endian()) {
      reverse_bytes(len);
    }
    p = RDFDictEncReader<ID, ENC>::readNext(is, len);
    RDFTerm term = RDFTerm::parse(p);
    p->drop();
    if (!dict->force(id, term)) {
      THROW(TraceableException, "Unable to map ID to term.");
    }
    p = RDFDictEncReader<ID, ENC>::readNext(is, ID::size() + sizeof(uint32_t));
  }
  return dict;
}

template<typename ID, typename ENC>
bool RDFDictEncReader<ID, ENC>::read(RDFTriple &triple) {
  ID id[3];
  while (this->readID(id[0])) {
    if (!this->readID(id[1]) || !this->readID(id[2])) {
      THROW(IOException, "Unexpected end of file.");
    }
    RDFTerm subj, pred, obj;
    if (!this->dict->lookup(id[0], subj) || !this->dict->lookup(id[1], pred) ||
        !this->dict->lookup(id[2], obj)) {
      THROW(TraceableException, "No such ID found in dictionary.");
    }
    bool atypical = (subj.isLiteral() || pred.getType() != IRI);
    if (atypical) {
      if (!this->discard_atypical) {
        THROW(TraceableException, "Encountered atypical literal.");
      }
    } else {
      // TODO add swap method to objects and use swap instead
      triple = RDFTriple(subj, pred, obj);
      return true;
    }
  }
  return false;
}

template<typename ID, typename ENC>
void RDFDictEncReader<ID, ENC>::close() {
  try {
    this->input->close();
  } JUST_RETHROW(IOException, "Trouble closing in RDFDictEncReader.")
}

template<typename ID, typename ENC>
bool RDFDictEncReader<ID, ENC>::readID(ID &id) {
  uint8_t *p = id.ptr();
  uint8_t *end = p + ID::size();
  while (this->buffer != NULL && p != end) {
    size_t amount = min(this->buffer->size() - this->offset, (size_t)(end - p));
    memcpy(p, this->buffer->dptr() + this->offset, amount);
    p += amount;
    this->offset += amount;
    if (this->offset >= this->buffer->size()) {
      this->buffer->drop();
      this->buffer = this->input->read();
      this->offset = 0;
    }
  }
  if (p != id.ptr() && p != end) {
    THROW(IOException, "Unexpected end of file.");
  }
  return p == end;
}

template<typename ID, typename ENC>
RDFDictionary<ID, ENC> *RDFDictEncReader<ID, ENC>::getDictionary() {
  return this->dict;
}

}
