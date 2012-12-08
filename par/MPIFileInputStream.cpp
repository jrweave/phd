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

#include "par/MPIFileInputStream.h"

#include "ptr/MPtr.h"

namespace par {

MPIFileInputStream::MPIFileInputStream(const MPI::File &f,
    const size_t page_size)
    throw(BadAllocException, TraceableException)
    : InputStream(), offset(0), length(0) {
  if (page_size <= 0) {
    THROW(TraceableException, "page_size must be positive.");
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
  this->file = f;
}

MPIFileInputStream::MPIFileInputStream(const MPI::Intracomm &comm,
    const char *filename, int amode, const MPI::Info &info,
    const size_t page_size)
    throw(IOException, BadAllocException, TraceableException)
    : InputStream(), offset(0), length(0) {
  if (page_size <= 0) {
    THROW(TraceableException, "page_size must be positive.");
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

DPtr<uint8_t> *MPIFileInputStream::read() 
    throw(IOException, BadAllocException) {
  try {
    int64_t avail = this->available();
    return this->read(avail == 0 ? 1 : avail);
  } JUST_RETHROW(IOException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
}

DPtr<uint8_t> *MPIFileInputStream::read(const int64_t amount)
    THROWS(IOException, BadAllocException) {
  if (this->offset == this->length) {
    this->fillBuffer();
    if (this->offset == this->length) {
      return NULL;
    }
  }
  int64_t avail = this->available();
  int64_t actual = min(amount, avail);
  DPtr<uint8_t> *part = this->buffer->sub(this->offset, actual);
  if (actual == avail) {
    try {
      part = part->stand();
    } JUST_RETHROW(BadAllocException, "(rethrow)")
    try {
      this->fillBuffer();
    } catch (IOException &e) {
      part->drop();
      RETHROW(e, "(rethrow)");
    }
  } else {
    this->offset += actual;
  }
  return part;
}
TRACE(IOException, "(trace)")

int64_t MPIFileInputStream::skip(const int64_t n) throw(IOException) {
  int64_t avail;
  try {
    avail = this->available();
  } JUST_RETHROW(BadAllocException, "(rethrow)")
  int64_t actual = min(n, avail);
  if (actual == avail) {
    try {
      this->fillBuffer();
    } JUST_RETHROW(IOException, "(rethrow)")
  } else {
    this->offset += actual;
  }
  return actual;
}

}
