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

#include "par/MPIDelimFileInputStream.h"

#include <vector>
#include "ptr/MPtr.h"

namespace par {

MPIDelimFileInputStream::MPIDelimFileInputStream(const MPI::Intracomm &comm,
    const MPI::File &f, const size_t page_size, const uint8_t delimiter)
    throw(BadAllocException, TraceableException)
    : MPIFileInputStream(f, page_size), delim(delimiter) {
  this->initialize(comm, page_size);
}

MPIDelimFileInputStream::MPIDelimFileInputStream(const MPI::Intracomm &comm,
    const char *filename, int amode, const MPI::Info &info,
    const size_t page_size, const uint8_t delimiter)
    throw(IOException, BadAllocException, TraceableException)
    : MPIFileInputStream(comm, filename, amode, info, page_size),
      marked(false), delim(delimiter) {
  this->initialize(comm, page_size);
}

void MPIDelimFileInputStream::initialize(const MPI::Intracomm &comm,
    const size_t page_size) {
  try {
    NEW(this->asyncbuf, MPtr<uint8_t>, page_size);
  } catch (bad_alloc &e) {
    //this->file.Close();
    THROW(BadAllocException, page_size*sizeof(uint8_t));
  } catch (BadAllocException &e) {
    //this->file.Close();
    RETHROW(e, "(rethrow)");
  }
  int rank = comm.Get_rank();
  int size = comm.Get_size();
  MPI::Offset filesize = this->file.Get_size();
  MPI::Offset num_pages = filesize / page_size;
  MPI::Offset pages_per_proc = num_pages / size;
  MPI::Offset extra_pages = num_pages % size;
  this->begin = rank * pages_per_proc * page_size;
  if (rank < extra_pages) {
    this->begin += rank*page_size;
  } else {
    this->begin += extra_pages*page_size;
  }
  this->at = this->begin;
  if (rank == size - 1) {
    this->end = filesize; // last processor gets dangling bytes
  } else {
    this->end = this->begin + pages_per_proc * page_size;
    if (rank < extra_pages) {
      this->end += page_size;
    }
  }
  bool found_delim = rank == 0 || rank == size - 1;
  MPI::Offset adjust = 0;
  MPI::Offset myadjust = 0;
  MPI::Status stat;
  if (this->at < this->end) {
    MPI::Offset amount = min(this->end - this->at,
                             (MPI::Offset) this->buffer->size());
    try {
      this->file.Read_at(this->at, this->buffer->ptr(), amount, MPI::BYTE,
                         stat);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
    this->offset = 0;
    this->length = stat.Get_count(MPI::BYTE);
    this->at += this->length;
    if (this->at < this->end) {
      amount = min(this->end - this->at,
                   (MPI::Offset) this->asyncbuf->size());
      if (!this->asyncbuf->alone()) {
        this->asyncbuf = this->asyncbuf->stand();
      }
      try {
        this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                        MPI::BYTE);
      } catch (MPI::Exception &e) {
        THROW(IOException, e.Get_error_string());
      }
    }
    if (rank > 0) {
      const uint8_t *p;
      const uint8_t *q;
      do {
        p = this->buffer->dptr();
        q = p + this->length;
        for (; p != q && *p != this->delim; ++p) {
          // find first delimiter
        }
        this->offset = p - this->buffer->dptr();
        adjust += this->offset;
        if (p != q) {
          found_delim = true;
          this->offset = 0;
          this->length = q - p - 1;
          memmove(this->buffer->dptr(), p + 1, this->length * sizeof(uint8_t));
          ++adjust;
        }
        if (p == q || p + 1 == q) {
          if (this->at >= this->end) {
            break;
          }
          try {
            while (!this->req.Test(stat)) {
              // wait for async comm to complete or cancel
            }
          } catch (MPI::Exception &e) {
            THROW(IOException, e.Get_error_string());
          }
          swap(this->buffer, this->asyncbuf);
          this->offset = 0;
          this->length = stat.Get_count(MPI::BYTE);
          this->at += this->length;
          if (this->at < this->end) {
            amount = min(this->end - this->at,
                         (MPI::Offset) this->asyncbuf->size());
            if (!this->asyncbuf->alone()) {
              this->asyncbuf = this->asyncbuf->stand();
            }
            try {
              this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(),
                                              amount, MPI::BYTE);
            } catch (MPI::Exception &e) {
              THROW(IOException, e.Get_error_string());
            }
          }
        }
      } while (p == q);
      this->begin += adjust;
    }
  }
  int send_to = rank == 0 ? size - 1 : rank - 1;
  int recv_from = rank == size - 1 ? 0 : rank + 1;
  try {
    MPI::Request recvreq = comm.Irecv(&myadjust, sizeof(MPI::Offset),
                                      MPI::BYTE, recv_from, 0);
    if (!found_delim) {
      while (!recvreq.Test()) {
        // wait to get adjustment
      }
      adjust += myadjust;
      myadjust = 0;
    }
    MPI::Request sendreq = comm.Isend(&adjust, sizeof(MPI::Offset),
                                      MPI::BYTE, send_to, 0);
    if (found_delim) {
      while (!recvreq.Test()) {
        // wait to get adjustment
      }
    }
    while  (!sendreq.Test()) {
      // wait for send to complete
    }
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  if (this->at == this->end && myadjust > 0) {
    MPI::Offset amount = min(myadjust, (MPI::Offset) this->asyncbuf->size());
    if (!this->asyncbuf->alone()) {
      this->asyncbuf = this->asyncbuf->stand();
    }
    try {
      this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                      MPI::BYTE);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  this->end += myadjust;
}

MPIDelimFileInputStream::~MPIDelimFileInputStream() throw(IOException) {
  if (this->at < this->end) {
    this->req.Cancel();
    while (!this->req.Test()) {
      // wait for async comm to complete or cancel
    }
  }
  this->asyncbuf->drop();
}

void MPIDelimFileInputStream::reset() throw(IOException) {
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
    this->asyncbuf = this->asyncbuf->stand();
  }
  try {
    this->req = this->file.Iread_at(this->at, this->asyncbuf->ptr(), amount,
                                    MPI::BYTE);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

DPtr<uint8_t> *MPIDelimFileInputStream::readDelimited()
    THROWS(IOException, BadAllocException) {
  if (this->offset == this->length) {
    this->fillBuffer();
    if (this->offset == this->length) {
      return NULL;
    }
  }
  const uint8_t *begin = this->buffer->dptr() + this->offset;
  const uint8_t *end = this->buffer->dptr() + this->length;
  const uint8_t *p = begin;
  for (; p != end && *p != this->delim; ++p) {
    // find next delimiter
  }
  if (p != end) {
    DPtr<uint8_t> *ret = this->buffer->sub(this->offset, p - begin);
    this->offset += p - begin + 1; // skips over delimiter
    return ret;
  }
  vector<void*> partials;
  size_t total_len = 0;
  DPtr<uint8_t> *part = this->buffer->sub(this->offset,
                                          this->length - this->offset);
  total_len += part->size();
  partials.push_back((void*) part);
  do {
    this->fillBuffer();
    if (this->offset == this->length) {
      break;
    }
    begin = this->buffer->dptr() + this->offset;
    end = this->buffer->dptr() + this->length;
    p = begin;
    for (; p != end && *p != this->delim; ++p) {
      // find next delimiter
    }
    part = this->buffer->sub(this->offset, p - begin);
    this->offset += p - begin + (p == end ? 0 : 1); // skips over delimiter
    total_len += part->size();
    partials.push_back((void*) part);
  } while (p == end);
  DPtr<uint8_t> *ret;
  try {
    NEW(ret, MPtr<uint8_t>, total_len);
  } catch (bad_alloc &e) {
    vector<void*>::iterator it = partials.begin();
    for (; it != partials.end(); ++it) {
      ((DPtr<uint8_t>*)(*it))->drop();
    }
    THROW(BadAllocException, sizeof(MPtr<uint8_t>));
  } catch (BadAllocException &e) {
    vector<void*>::iterator it = partials.begin();
    for (; it != partials.end(); ++it) {
      ((DPtr<uint8_t>*)(*it))->drop();
    }
    RETHROW(e, "(rethrow)");
  }
  uint8_t *write_to = ret->dptr();
  vector<void*>::iterator it = partials.begin();
  for (; it != partials.end(); ++it) {
    part = (DPtr<uint8_t>*) (*it);
    memcpy(write_to, part->dptr(), part->size() * sizeof(uint8_t));
    write_to += part->size();
    part->drop();
  }
  return ret;
}
TRACE(IOException, "(trace)")

void MPIDelimFileInputStream::fillBuffer() throw(IOException) {
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
      this->asyncbuf = this->asyncbuf->stand();
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
