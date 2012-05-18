#include "par/MPIFileInputStream.h"

namespace par { 

inline
MPIFileInputStream::~MPIFileInputStream() throw(IOException) {
  this->buffer->drop();
}

inline
int64_t MPIFileInputStream::available() throw(IOException) {
  return (int64_t)(this->length - this->offset);
}

inline
void MPIFileInputStream::close() throw(IOException) {
  try {
    this->file.Close();
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

}
