#include "io/InputStream.h"

namespace io {

DPtr<uint8_t> *InputStream::read() 
    throw(IOException, BadAllocException) {
  try {
    int64_t avail = this->available();
    return this->read(avail == 0 ? 1 : avail);
  } JUST_RETHROW(IOException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
}

int64_t InputStream::skip(const int64_t n) throw(IOException) {
  int64_t i = 0;
  for (; i < n; ++i) {
    if (this->read() < INT16_C(0)) {
      return i == 0 ? INT64_C(-1) : i;
    }
  }
  return i;
}

}
