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

#include "par/MPIPartialFileInputStream.h"

#include "ptr/MPtr.h"

namespace par {

MPIPartialFileInputStream::MPIPartialFileInputStream(const MPI::File &f,
    const size_t page_size, const MPI::Offset begin, const MPI::Offset end)
    throw(BadAllocException, TraceableException)
    : MPIFileInputStream(f, page_size), begin(begin), at(begin), end(end),
      marker(begin), asyncbuf(NULL) {
  try {
    this->initialize(page_size);
  } JUST_RETHROW(BadAllocException,
                 "Problem constructing MPIPartialFileInputStream.")
    JUST_RETHROW(TraceableException,
                 "Problem constructing MPIPartialFileInputStream.")
}

MPIPartialFileInputStream::MPIPartialFileInputStream(
    const MPI::Intracomm &comm, const char *filename, int amode,
    const MPI::Info &info, const size_t page_size, const MPI::Offset begin,
    const MPI::Offset end)
    throw(IOException, BadAllocException, TraceableException)
    : MPIFileInputStream(comm, filename, amode, info, page_size), begin(begin),
      at(begin), end(end), marker(begin), asyncbuf(NULL) {
  try {
    this->initialize(page_size);
  } JUST_RETHROW(BadAllocException,
                 "Problem constructing MPIPartialFileInputStream.")
    JUST_RETHROW(TraceableException,
                 "Problem constructing MPIPartialFileInputStream.")
}

void MPIPartialFileInputStream::initialize(const size_t page_size) {
  if (this->end < 0) {
    this->end = this->file.Get_size();
  }
  if (this->begin > this->end) {
    THROW(TraceableException, "begin must be <= end.");
  }
  try {
    NEW(this->asyncbuf, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
  this->offset = 0;
  this->length = 0;
  MPI::Offset amount = this->asyncbuf->size()
                       - (this->at % this->asyncbuf->size());
  amount = min(amount, this->end - this->at);
  try {
    this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                    MPI::BYTE);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

MPIPartialFileInputStream::~MPIPartialFileInputStream() throw(IOException) {
  if (this->at < this->end) {
    try {
      this->req.Cancel();
      while (!this->req.Test()) {
        // wait for async comm to complete or cancel
      }
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  this->asyncbuf->drop();
}

bool MPIPartialFileInputStream::mark(const int64_t read_limit) throw(IOException) {
  this->marker = this->at + this->offset;
  return true;
}

bool MPIPartialFileInputStream::markSupported() const throw() {
    return true;
}

void MPIPartialFileInputStream::reset() throw(IOException) {
  if (this->at - this->marker < this->length) {
    this->offset = this->length - (this->at - this->marker);
    return;
  }
  if (this->at < this->end) {
    this->req.Cancel();
    while (!this->req.Test()) {
      // wait for async comm to complete or cancel
    }
  }
  this->offset = 0;
  this->length = 0;
  this->at = this->marker;
  if (this->at >= this->end) {
    return;
  }
  MPI::Offset amount = this->asyncbuf->size()
                       - (this->at % this->asyncbuf->size());
  amount = min(amount, this->end - this->at);
  if (!this->asyncbuf->alone()) {
    DPtr<uint8_t> *p = NULL;
    try {
      NEW(p, MPtr<uint8_t>, this->asyncbuf->size());
    } catch (BadAllocException &e) {
      THROW(IOException, e.what());
    }
    this->asyncbuf->drop();
    this->asyncbuf = p;
  }
  try {
    this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                    MPI::BYTE);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

void MPIPartialFileInputStream::fillBuffer() throw(IOException) {
  if (this->at == this->end) {
    this->offset = 0;
    this->length = 0;
    return;
  }
  MPI::Status stat;
  try {
    while (!this->req.Test(stat)) {
      // wait for async comm to complete
    }
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  swap(this->buffer, this->asyncbuf);
  this->offset = 0;
  this->length = stat.Get_count(MPI::BYTE);
  this->at += this->length;
  if (this->at < this->end) {
    MPI::Offset amount = min((MPI::Offset) this->asyncbuf->size(),
                             this->end - this->at);
    if (!this->asyncbuf->alone()) {
      DPtr<uint8_t> *p = NULL;
      try {
        NEW(p, MPtr<uint8_t>, this->asyncbuf->size());
      } catch (BadAllocException &e) {
        THROW(IOException, e.what());
      }
      this->asyncbuf->drop();
      this->asyncbuf = p;
    }
    try {
      this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                      MPI::BYTE);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
}

}
