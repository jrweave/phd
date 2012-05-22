#include "par/MPIFileOutputStream.h"

namespace par {

inline
MPIFileOutputStream::~MPIFileOutputStream() throw(IOException) {
  this->buffer->drop();
}

inline
void MPIFileOutputStream::close() throw(IOException) {
  try {
    this->file.Close();
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

inline
void MPIFileOutputStream::flush() THROWS(IOException) {
  if (this->length > 0) {
    this->writeBuffer();
    this->length = 0;
  }
}
TRACE(IOException, "Error when flushing.")

}
