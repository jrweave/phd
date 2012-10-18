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

#include "io/OStream.h"

#include "sys/char.h"

namespace io {

OStream<ostream>::OStream() throw()
    : OutputStream(), stream(cout) {
  // do nothing
}

OStream<ostream>::OStream(ostream &stream) throw()
    : OutputStream(), stream(stream) {
  // do nothing
}

OStream<ostream>::~OStream() throw(IOException) {
  // do nothing
}

void OStream<ostream>::close() throw(IOException) {
  // ostream has no close method;
  // at least flush the buffer
  this->stream.flush();
  if (this->stream.bad()) {
    THROW(IOException, "Failed to flush ostream when close was called.");
  }
}

void OStream<ostream>::flush() throw(IOException) {
  this->stream.flush();
  if (this->stream.bad()) {
    THROW(IOException, "Failed to flush ostream.");
  }
}

void OStream<ostream>::write(DPtr<uint8_t> *buf, size_t &nwritten)
    throw(IOException, SizeUnknownException, BaseException<void*>) {
  if (buf == NULL) {
    THROW(BaseException<void*>, NULL, "buf must not be NULL.");
  }
  if (!buf->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  const uint8_t *mark = buf->dptr();
  const uint8_t *end = mark + buf->size();
  for (; mark != end; ++mark) {
    this->stream.put(to_lchar(*mark));
    if (this->stream.bad()) {
      nwritten = mark - buf->dptr();
      THROW(IOException, "Error when writing to underlying stream.");
    }
  }
  nwritten = mark - buf->dptr();
}

streampos OStream<ostream>::tellp() throw(IOException) {
  return this->stream.tellp();
}

void OStream<ostream>::seekp(streampos pos) throw(IOException) {
  this->stream.seekp(pos);
  if (this->stream.fail()) {
    THROW(IOException, "Could not reach specified seek position.");
  }
  if (this->stream.bad()) {
    THROW(IOException, "Error in seeking underlying ostream.");
  }
}

void OStream<ostream>::seekp(streamoff off, ios_base::seekdir dir)
    throw(IOException) {
  this->stream.seekp(off, dir);
  if (this->stream.fail()) {
    THROW(IOException, "Could not reach specified seek position.");
  }
  if (this->stream.bad()) {
    THROW(IOException, "Error in seeking underlying ostream.");
  }
}

}
