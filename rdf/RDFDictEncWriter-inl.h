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

#include "rdf/RDFDictEncWriter.h"

#include <algorithm>
#include "ptr/MPtr.h"

namespace rdf {

using namespace std;

template<typename ID, typename ENC>
RDFDictEncWriter<ID, ENC>::RDFDictEncWriter(OutputStream *os,
    RDFDictionary<ID, ENC> *dict, const bool own_dictionary)
    throw(BaseException<void*>, BadAllocException)
    : output(os), dict(dict), buffer(NULL),
      own_dictionary(own_dictionary) {
  if (os == NULL) {
    THROW(BaseException<void*>, NULL, "os must not be NULL.");
  }
  if (dict == NULL) {
    try {
      NEW(dict, WHOLE(RDFDictionary<ID, ENC>));
    } RETHROW_BAD_ALLOC
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, 3*ID::size());
  } RETHROW_BAD_ALLOC
}

template<typename ID, typename ENC>
RDFDictEncWriter<ID, ENC>::~RDFDictEncWriter() THROWS(IOException) {
  DELETE(this->output);
  if (this->own_dictionary) {
    DELETE(dict);
  }
  this->buffer->drop();
}
TRACE(IOException, "Trouble deconstructing RDFDictEncWriter.")

template<typename ID, typename ENC>
void RDFDictEncWriter<ID, ENC>::writeDictionary(OutputStream *os,
    RDFDictionary<ID, ENC> *dict) {
  ID noflip(0);
  RDFDictEncWriter<ID, ENC>::writeDictionary(os, dict, noflip);
}

template<typename ID, typename ENC>
void RDFDictEncWriter<ID, ENC>::writeDictionary(OutputStream *os,
    RDFDictionary<ID, ENC> *dict, const ID &bitflip) {
  DPtr<uint8_t> *p;
  try {
    NEW(p, MPtr<uint8_t>, max((int)(ID::size() + sizeof(uint32_t)), 1024));
  } RETHROW_BAD_ALLOC
  typename RDFDictionary<ID, ENC>::const_iterator it = dict->begin();
  typename RDFDictionary<ID, ENC>::const_iterator end = dict->end();
  for (; it != end; ++it) {
    DPtr<uint8_t> *str = it->second.toUTF8String();
    if (str->size() > (size_t) UINT32_MAX) {
      p->drop();
      str->drop();
      THROW(TraceableException,
            "RDFTerm too long to write in dictionary file.");
    }
    if (p->size() < ID::size() + sizeof(uint32_t) + str->size()) {
      p->drop();
      try {
        NEW(p, MPtr<uint8_t>, ID::size() + sizeof(uint32_t) + str->size());
      } RETHROW_BAD_ALLOC
    } else if (!p->alone()) {
      p = p->stand(false);
    }
    ID flipped(it->first);
    flipped ^= bitflip;
    memcpy(p->dptr(), flipped.ptr(), ID::size());
    uint32_t len = (uint32_t) str->size();
    if (is_little_endian()) {
      reverse_bytes(len);
    }
    memcpy(p->dptr() + ID::size(), &len, sizeof(uint32_t));
    // I could just write p with the header and the str afterward,
    // but then there would be two calls to the underlying OutputStream.
    // This can make a difference if splitting is prevented in a parallel
    // context.  I shouldn't have to account for that here, but doing so
    // saves me some time... for now.
    memcpy(p->dptr() + ID::size() + sizeof(uint32_t), str->dptr(), str->size());
    DPtr<uint8_t> *subp = p->sub(0, ID::size() + sizeof(uint32_t) + str->size());
    str->drop();
    os->write(subp);
    subp->drop();
  }
  p->drop();
}

template<typename ID, typename ENC>
void RDFDictEncWriter<ID, ENC>::write(const RDFTriple &triple) {
  ID encoded_triple[3];
  try {
    encoded_triple[0] = this->dict->encode(triple.getSubj());
    encoded_triple[1] = this->dict->encode(triple.getPred());
    encoded_triple[2] = this->dict->encode(triple.getObj());
  } JUST_RETHROW(TraceableException, "Unable to encode triple.")
  if (!this->buffer->alone()) {
    this->buffer = this->buffer->stand(false);
  }
  uint8_t *p = this->buffer->dptr();
  memcpy(p, encoded_triple[0].ptr(), ID::size());
  p += ID::size();
  memcpy(p, encoded_triple[1].ptr(), ID::size());
  p += ID::size();
  memcpy(p, encoded_triple[2].ptr(), ID::size());
  try {
    this->output->write(this->buffer);
  } JUST_RETHROW(IOException, "Unable to write.")
}

template<typename ID, typename ENC>
void RDFDictEncWriter<ID, ENC>::close() {
  try {
    this->output->close();
  } JUST_RETHROW(IOException, "Trouble closing in RDFDictEncWriter.")
}

template<typename ID, typename ENC>
RDFDictionary<ID, ENC> *RDFDictEncWriter<ID, ENC>::getDictionary() {
  return this->dict;
}

}
