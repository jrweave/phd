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

#include "par/MPIFileOutputStream.h"

#include "ptr/MPtr.h"

namespace par {

// TODO: for some reason, using this constructor seems to make
// TODO: the object not work; fix this some time
MPIFileOutputStream::MPIFileOutputStream(const MPI::File &f,
    const size_t page_size, const bool no_splitting)
    throw(BadAllocException, TraceableException)
    : OutputStream(), length(0), no_splitting(no_splitting) {
  if (page_size <= 0) {
    THROW(TraceableException, "Specified page size must be positive.");
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
  this->file = file;
}

MPIFileOutputStream::MPIFileOutputStream(const MPI::Intracomm &comm,
    const char *filename, int amode, const MPI::Info &info,
    const size_t page_size, const bool no_splitting)
    throw(IOException, BadAllocException, TraceableException)
    : OutputStream(), length(0), no_splitting(no_splitting) {
  if (page_size <= 0) {
    THROW(TraceableException, "Specified page size must be positive.");
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
  try {
    this->file = MPI::File::Open(comm, filename, amode, info);
  } catch (MPI::Exception &e) {
    this->buffer->drop();
    THROW(IOException, e.Get_error_string());
  }
}

void MPIFileOutputStream::write(DPtr<uint8_t> *buf)
    throw(IOException, SizeUnknownException, BaseException<void*>) {
  size_t nwritten;
  try {
    this->write(buf, nwritten);
  } JUST_RETHROW(IOException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BaseException<void*>, "(rethrow)")
}

void MPIFileOutputStream::write(DPtr<uint8_t> *buf, size_t &nwritten)
    throw(IOException, SizeUnknownException, BaseException<void*>) {
  if (buf == NULL) {
    THROW(BaseException<void*>, NULL, "buf must not be NULL.");
  }
  if (!buf->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  nwritten = 0;
  if (this->no_splitting) {
    if (buf->size() > this->buffer->size()) {
      THROW(IOException,
          "Requested write of unsplitable buffer that is larger than a page.");
    }
    if (buf->size() > this->buffer->size() - this->length) {
      try {
        this->writeBuffer();
      } JUST_RETHROW(IOException, "Couldn't write partially filled buffer.");
      this->length = 0;
    }
  }
  const uint8_t *b = buf->dptr();
  const uint8_t *e = b + buf->size();
  while (b != e) {
    size_t amount = min((size_t)(e-b), this->buffer->size() - this->length);
    memcpy(this->buffer->dptr() + this->length,
           b, amount * sizeof(uint8_t));
    this->length += amount;
    b += amount;
    if (this->length >= this->buffer->size()) {
      try {
        this->writeBuffer();
      } JUST_RETHROW(IOException, "Couldn't write filled buffer.");
      this->length = 0;
    }
  }
}

}
