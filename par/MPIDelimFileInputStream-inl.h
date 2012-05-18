#include "par/MPIDelimFileInputStream.h"

namespace par {

inline
bool MPIDelimFileInputStream::mark(const int64_t read_limit)
    throw(IOException) {
  this->marker = this->at - this->buffer->size() + this->offset;
  return true;
}

inline
bool MPIDelimFileInputStream::markSupported() const throw() {
  return true;
}

}
