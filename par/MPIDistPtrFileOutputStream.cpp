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

#include "par/MPIDistPtrFileOutputStream.h"

#include "ptr/MPtr.h"

#define PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG 0

namespace par {

MPIDistPtrFileOutputStream::MPIDistPtrFileOutputStream(
    MPI::Intracomm &comm, const MPI::File &file,
    const size_t page_size, const bool no_splitting)
    throw(BadAllocException, TraceableException)
    : MPIFileOutputStream(file, page_size, no_splitting), comm(comm),
      bytes_written(0), last_file_length(0), started(false) {
  try {
    NEW(this->asyncbuf, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
}

MPIDistPtrFileOutputStream::MPIDistPtrFileOutputStream(
    MPI::Intracomm &comm, const char *filename, int amode,
    const MPI::Info &info, const size_t page_size, const bool no_splitting)
    throw(IOException, BadAllocException, TraceableException)
    : MPIFileOutputStream(comm, filename, amode, info, page_size,
      no_splitting), comm(comm), bytes_written(0), last_file_length(0),
      started(false) {
  try {
    NEW(this->asyncbuf, MPtr<uint8_t>, page_size);
  } RETHROW_BAD_ALLOC
}

MPIDistPtrFileOutputStream::~MPIDistPtrFileOutputStream() throw(IOException) {
  this->asyncbuf->drop();
}

void MPIDistPtrFileOutputStream::close() throw(IOException) {
  if (this->length > 0) {
    try {
      this->flush();
    } JUST_RETHROW(IOException, "Couldn't flush.")
    this->length = 0;
  }

  // Need to do fake/empty writes to coordinate with other processors until
  // all processors are done
  unsigned long flen = this->last_file_length;
  try {
    this->comm.Allreduce(&this->bytes_written, &this->last_file_length, 1,
                         MPI::UNSIGNED_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  while (!this->started || flen != this->last_file_length) {
    unsigned long len = (unsigned long) this->length;
    unsigned long offset;
    try {
      this->comm.Scan(&len, &offset, 1, MPI::UNSIGNED_LONG, MPI::SUM);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
    offset -= len;
    if (this->started) {
      MPI::Status stat;
      try {
#if PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG
        cerr << __FILE__ << ':' << __LINE__ << " [" << MPI::COMM_WORLD.Get_rank() << "] Write_at_all_end(" << (void*)this->asyncbuf->dptr() << ", stat)" << endl;
#endif
        this->file.Write_at_all_end(this->asyncbuf->dptr(), stat);
      } catch (MPI::Exception &e) {
        THROW(IOException, e.Get_error_string());
      }
    }
    try {
#if PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG
      cerr << __FILE__ << ':' << __LINE__ << " [" << MPI::COMM_WORLD.Get_rank() << "] Write_at_all_begin("  << (MPI::Offset)(this->last_file_length + offset) << ", " << (void*)this->asyncbuf->dptr() << ", " << this->length << ", MPI::BYTE)" << endl;
#endif
      this->file.Write_at_all_begin(
          (MPI::Offset)(this->last_file_length + offset), this->asyncbuf->dptr(),
          this->length, MPI::BYTE);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
    this->bytes_written += this->length;
    this->started = true;
    flen = this->last_file_length;
    try {
      this->comm.Allreduce(&this->bytes_written, &this->last_file_length, 1,
                           MPI::UNSIGNED_LONG, MPI::SUM);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  try {
    MPI::Status stat;
#if PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG
    cerr << __FILE__ << ':' << __LINE__ << " [" << MPI::COMM_WORLD.Get_rank() << "] Write_at_all_end(" << (void*)this->asyncbuf->dptr() << ", stat)" << endl;
#endif
    this->file.Write_at_all_end(this->asyncbuf->dptr(), stat);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  try {
    MPIFileOutputStream::close();
  } JUST_RETHROW(IOException, "Couldn't close.")
}

size_t MPIDistPtrFileOutputStream::writeBuffer() throw(IOException) {
  try {
    this->comm.Allreduce(&this->bytes_written, &this->last_file_length, 1,
                         MPI::UNSIGNED_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  unsigned long len = (unsigned long) this->length;
  unsigned long offset;
  try {
    this->comm.Scan(&len, &offset, 1, MPI::UNSIGNED_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  offset -= len;
  if (this->started) {
    MPI::Status stat;
    try {
#if PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG
      cerr << __FILE__ << ':' << __LINE__ << " [" << MPI::COMM_WORLD.Get_rank() << "] Write_at_all_end(" << (void*)this->asyncbuf->dptr() << ", stat)" << endl;
#endif
      this->file.Write_at_all_end(this->asyncbuf->dptr(), stat);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  swap(this->buffer, this->asyncbuf);
  try {
#if PAR_MPI_DIST_PTR_FILE_OUTPUT_STREAM_DEBUG
    cerr << __FILE__ << ':' << __LINE__ << " [" << MPI::COMM_WORLD.Get_rank() << "] Write_at_all_begin(" << (MPI::Offset)(this->last_file_length + offset) << ", " << (void*)this->asyncbuf->dptr() << ", " << this->length << ", MPI::BYTE)" << endl;
#endif
    this->file.Write_at_all_begin(
        (MPI::Offset)(this->last_file_length + offset), this->asyncbuf->dptr(),
        this->length, MPI::BYTE);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  this->bytes_written += this->length;
  this->started = true;
  return 0;
}

}
