#include "io/InputStream.h"

namespace io {

inline
InputStream::~InputStream() throw(IOException) {
  // do nothing
}

inline
int64_t InputStream::available() throw(IOException) {
  return INT64_C(0);
}

inline
bool InputStream::mark(const int64_t read_limit) throw(IOException) {
  return false;
}

inline
bool InputStream::markSupported() const throw() {
  return false;
}

inline
void InputStream::reset() throw(IOException) {
  THROW(IOException, "reset is not supported.");
}

}
