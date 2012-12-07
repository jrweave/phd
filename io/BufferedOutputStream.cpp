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

#include "io/BufferedOutputStream.h"

#include "ptr/MPtr.h"

namespace io {

BufferedOutputStream::BufferedOutputStream(OutputStream *os,
    const size_t bufsize, const bool allow_splitting)
    throw(BaseException<void*>, BadAllocException)
    : buffer(NULL), output_stream(os), length(0),
      allow_splitting(allow_splitting) {
  if (os == NULL) {
    THROW(BaseException<void*>, NULL, "os must not be NULL.");
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, bufsize);
  } RETHROW_BAD_ALLOC
}

BufferedOutputStream::~BufferedOutputStream() THROWS(IOException) {
  DELETE(this->output_stream);
  this->buffer->drop();
}
TRACE(IOException, "Problem deconstructing BufferedOutputStream.")

void BufferedOutputStream::close() THROWS(IOException) {
  this->flush();
  this->output_stream->close();
}
TRACE(IOException, "Problem closing BufferedOutputStream.")

void BufferedOutputStream::flush() THROWS(IOException) {
  this->writeBuffer();
  this->output_stream->flush();
}
TRACE(IOException, "Problem flushing BufferedOutputStream.")

void BufferedOutputStream::write(DPtr<uint8_t> *buf, size_t &nwritten)
    THROWS(IOException, SizeUnknownException, BaseException<void*>) {
  if (buf == NULL) {
    THROW(BaseException<void*>, NULL, "buf must not be NULL.");
  }
  if (!buf->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  nwritten = 0;
  if (this->buffer->size() - this->length >= buf->size()) {
    memcpy(this->buffer->dptr() + this->length, buf->dptr(), buf->size());
    this->length += buf->size();
    if (this->length >= this->buffer->size()) {
      this->writeBuffer();
    }
    return;
  }
  if (!this->allow_splitting) {
    this->writeBuffer();
    if (this->buffer->size() < buf->size()) {
      this->output_stream->write(buf, nwritten);
      return;
    }
  }
  size_t nwrite = buf->size();
  uint8_t *b = buf->dptr() + (buf->size() - nwrite);
  size_t nopen = this->buffer->size() - this->length;
  while (nopen <= nwrite) {
    memcpy(this->buffer->dptr() + this->length, b, nopen);
    this->length += nopen;
    this->writeBuffer();
    nwrite -= nopen;
    b += nopen;
    nopen = this->buffer->size() - this->length;
  }
  memcpy(this->buffer->dptr() + this->length, b, nwrite);
  this->length += nwrite;
}
TRACE(IOException, "Problem writing in BufferedOutputStream.")

void BufferedOutputStream::writeBuffer() THROWS(IOException) {
  if (this->length <= 0) {
    return;
  }
  if (this->length >= this->buffer->size()) {
    this->output_stream->write(this->buffer);
  } else {
    DPtr<uint8_t> *p = this->buffer->sub(0, this->length);
    this->output_stream->write(p);
    p->drop();
  }
  this->length = 0;
  if (!this->buffer->alone()) {
    if (this->buffer->standable()) {
      this->buffer = this->buffer->stand();
    } else {
      size_t len = this->buffer->size();
      this->buffer->drop();
      try {
        NEW(this->buffer, MPtr<uint8_t>, len);
      } catch (BadAllocException &e) {
        THROW(IOException, e.what());
      }
    }
  }
}
TRACE(IOException, "Problem writing buffer in BufferedOutputStream.")

}
