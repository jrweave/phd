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

#include "io/BufferedInputStream.h"

#include "ptr/MPtr.h"

namespace io {

BufferedInputStream::BufferedInputStream(InputStream *is, const size_t buflen)
    throw(BaseException<void*>, BadAllocException)
    : buffer(NULL), input_stream(is), offset(0), length(0) {
  if (is == NULL) {
    THROW(BaseException<void*>, NULL, "is must not be NULL.");
  }
  try {
    NEW(buffer, MPtr<uint8_t>, buflen);
  } RETHROW_BAD_ALLOC
}

BufferedInputStream::~BufferedInputStream() THROWS(IOException) {
  DELETE(this->input_stream);
  this->buffer->drop();
}
TRACE(IOException, "Problem deconstructing BufferedInputStream.")

int64_t BufferedInputStream::available() throw(IOException) {
  return this->length - this->offset;
}

void BufferedInputStream::close() THROWS(IOException) {
  this->input_stream->close();
}
TRACE(IOException, "Problem closing BufferedInputStream.")

DPtr<uint8_t> *BufferedInputStream::read()
    throw(BadAllocException, IOException) {
  try {
    return this->read(this->buffer->size());
  } JUST_RETHROW(BadAllocException, "Problem reading in BufferedInputStream.")
    JUST_RETHROW(IOException, "Problem reading in BufferedInputStream.")
}

DPtr<uint8_t> *BufferedInputStream::read(const int64_t amount)
    THROWS(IOException, BadAllocException) {
  if (this->length - this->offset < this->buffer->size()) {
    if (!this->fillBuffer()) {
      return NULL;
    }
  }
  if (this->length - this->offset >= amount) {
    DPtr<uint8_t> *p = this->buffer->sub(this->offset, amount);
    this->offset += amount;
    return p;
  }
  DPtr<uint8_t> *p;
  try {
    NEW(p, MPtr<uint8_t>, amount);
  } RETHROW_BAD_ALLOC
  uint8_t *q = p->dptr();
  const uint8_t *begin = q;
  const uint8_t *end = q + amount;
  memcpy(q, this->buffer->dptr() + this->offset, this->length - this->offset);
  q += (this->length - this->offset);
  this->offset = this->length;
  while (q != end) {
    if (!this->fillBuffer()) {
      if (q == begin) {
        p->drop();
        return NULL;
      }
      DPtr<uint8_t> *ret = p->sub(0, q - begin);
      p->drop();
      if (ret->standable()) {
        ret = ret->stand();
      }
      return ret;
    }
    size_t len = min((size_t)(end - q), this->length - this->offset);
    memcpy(q, this->buffer->dptr() + this->offset, len);
    q += len;
    this->offset += len;
  }
  return p;
}
TRACE(IOException, "Problem reading in BufferedInputStream.")

void BufferedInputStream::reset() THROWS(IOException) {
  this->input_stream->reset();
  offset = 0;
  length = 0;
}
TRACE(IOException, "Problem reseting BufferedInputStream.")

bool BufferedInputStream::fillBuffer() THROWS(IOException) {
  if (!this->buffer->alone() && this->buffer->standable()) {
    this->buffer = this->buffer->stand();
  }
  if (this->offset > 0) {
    memmove(this->buffer->dptr(), this->buffer->dptr() + this->offset,
            this->length - this->offset);
    this->length -= this->offset;
    this->offset = 0;
  }
  while (this->length < this->buffer->size()) {
    DPtr<uint8_t> *p = this->input_stream->read(
                           this->buffer->size() - this->length);
    if (p == NULL) {
      return this->length > 0;
    }
    memcpy(this->buffer->dptr() + this->length, p->dptr(), p->size());
    this->length += p->size();
    p->drop();
  }
  return true;
}
TRACE(IOException, "Problem filling buffer in BufferedInputStream.")

}
