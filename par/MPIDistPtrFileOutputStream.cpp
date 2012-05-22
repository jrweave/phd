#include "par/MPIDistPtrFileOutputStream.h"

#include "ptr/MPtr.h"

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
  unsigned long long flen = this->last_file_length;
  try {
    this->comm.Allreduce(&this->bytes_written, &this->last_file_length, 1,
                         MPI::UNSIGNED_LONG_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  while (flen != this->last_file_length) {
    unsigned long long len = (unsigned long long) this->length;
    unsigned long long offset;
    try {
      this->comm.Scan(&len, &offset, 1, MPI::UNSIGNED_LONG_LONG, MPI::SUM);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
    offset -= len;
    if (this->started) {
      MPI::Status stat;
      try {
        this->file.Write_at_all_end(this->asyncbuf->dptr(), stat);
      } catch (MPI::Exception &e) {
        THROW(IOException, e.Get_error_string());
      }
    }
    try {
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
                           MPI::UNSIGNED_LONG_LONG, MPI::SUM);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  try {
    MPIFileOutputStream::close();
  } JUST_RETHROW(IOException, "Couldn't close.")
}

size_t MPIDistPtrFileOutputStream::writeBuffer() throw(IOException) {
  try {
    this->comm.Allreduce(&this->bytes_written, &this->last_file_length, 1,
                         MPI::UNSIGNED_LONG_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  unsigned long long len = (unsigned long long) this->length;
  unsigned long long offset;
  try {
    this->comm.Scan(&len, &offset, 1, MPI::UNSIGNED_LONG_LONG, MPI::SUM);
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
  offset -= len;
  if (this->started) {
    MPI::Status stat;
    try {
      this->file.Write_at_all_end(this->asyncbuf->dptr(), stat);
    } catch (MPI::Exception &e) {
      THROW(IOException, e.Get_error_string());
    }
  }
  swap(this->buffer, this->asyncbuf);
  try {
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
