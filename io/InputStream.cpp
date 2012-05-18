#include "io/InputStream.h"

namespace io {

InputStream::~InputStream() throw(IOException) {
  // do nothing
}

int64_t InputStream::available() throw(IOException) {
  return INT64_C(0);
}

bool InputStream::mark(const int64_t read_limit) throw(IOException) {
  return false;
}

bool InputStream::markSupported() const throw() {
  return false;
}

DPtr<uint8_t> *InputStream::read() 
    throw(IOException, BadAllocException) {
  try {
    int64_t avail = this->available();
    return this->read(avail == 0 ? 1 : avail);
  } JUST_RETHROW(IOException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
}

void InputStream::reset() throw(IOException) {
  THROW(IOException, "reset is not supported.");
}

int64_t InputStream::skip(const int64_t n) throw(IOException) {
  int64_t i = 0;
  for (; i < n; ++i) {
    if (this->read() < INT16_C(0)) {
      return i == 0 ? INT64_C(-1) : 0;
    }
  }
  return i;
}

}
