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

#include "io/IStream.h"

#include <algorithm>
#include "ptr/MPtr.h"
#include "sys/char.h"

namespace io {

IStream<istream>::IStream(const size_t bufsize) throw(BadAllocException)
    : stream(cin), offset(0), length(0), marked(false) {
  try {
    NEW(this->buffer, MPtr<uint8_t>, bufsize);
  } RETHROW_BAD_ALLOC
  this->mark_support = (this->stream.tellg() != (streampos) -1);
}

IStream<istream>::IStream(istream &stream) throw(BadAllocException)
    : stream(stream), offset(0), length(0), marked(false) {
  try {
    NEW(this->buffer, MPtr<uint8_t>, 4096);
  } RETHROW_BAD_ALLOC
  this->mark_support = (this->stream.tellg() != (streampos) -1);
}

IStream<istream>::IStream(istream &stream, const size_t bufsize)
    throw(BadAllocException, BaseException<size_t>)
    : stream(stream), offset(0), length(0), marked(false) {
  if (bufsize <= 0) {
    THROW(BaseException<size_t>, 0, "Specified buffer size must be positive.");
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, bufsize);
  } RETHROW_BAD_ALLOC
  this->mark_support = (this->stream.tellg() != (streampos) -1);
}

IStream<istream>::~IStream() throw(IOException) {
  this->buffer->drop();
}

void IStream<istream>::close() throw(IOException) {
  // do nothing; istream has no close method
}

int64_t IStream<istream>::available() throw(IOException) {
  if (this->offset < this->length) {
    return this->length - this->offset;
  }
  return min(
    (size_t) (this->stream.rdbuf()->in_avail()
              * sizeof(char) / sizeof(uint8_t)),
    this->buffer->size());
}

bool IStream<istream>::mark(const int64_t read_limit) throw(IOException) {
  this->marked = true;
  this->marker = this->stream.tellg();
  return this->marker != (streampos) -1;
}

bool IStream<istream>::markSupported() const throw() {
  return this->mark_support;
}

DPtr<uint8_t> *IStream<istream>::read(const int64_t amount)
    throw(IOException, BadAllocException) {
  if (this->stream.eof()) {
    return NULL;
  }
  if (!this->stream.good()) {
    THROW(IOException, "Unknown stream error.");
  }
  int64_t avail = max(INT64_C(1), this->available());
  int64_t actual = min(avail, amount);
  if (this->offset >= this->length) {
    if (!this->buffer->alone()) {
      this->buffer = this->buffer->stand();
    }
    uint8_t *p = this->buffer->dptr();
    uint8_t *end = p + actual;
    for (; p != end; ++p) {
      char c = (char) this->stream.get();
      if (this->stream.eof()) {
        if (p == this->buffer->dptr()) {
          return NULL;
        }
        break;
      }
      if (this->stream.fail()) {
        THROW(IOException, "Failed input operation.");
      }
      if (this->stream.bad()) {
        THROW(IOException, "Bad operation on buffer underlying stream.");
      }
      *p = to_ascii(c);
    }
    DPtr<uint8_t> *s = this->buffer->sub(0, p - this->buffer->dptr());
    return s;
  }
  DPtr<uint8_t> *s = this->buffer->sub(this->offset, actual);
  this->offset += actual;
  return s;
}

void IStream<istream>::reset() throw(IOException) {
  if (!this->marked) {
    THROW(IOException, "A mark has not been set.");
  }
  this->stream.seekg(this->marker);
  if (this->stream.fail()) {
    THROW(IOException, "Underlying stream could not reset to mark.");
  }
  if (this->stream.bad()) {
    THROW(IOException, "Unknown stream error.");
  }
}

int64_t IStream<istream>::skip(const int64_t n) throw(IOException) {
  streampos at = this->stream.tellg();
  this->stream.ignore(n);
  if (this->stream.bad()) {
    THROW(IOException, "Unknown stream error.");
  }
  return this->stream.tellg() - at;
}

}
