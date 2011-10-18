#include "ptr/BadAllocException.h"

#include <sstream>

namespace ptr {

using namespace std;

BadAllocException::BadAllocException(const char *file,
    const unsigned int line) throw()
    : TraceableException(file, line, "Bad Allocation"), size(0),
      size_known(false) {
  // do nothing
}

BadAllocException::BadAllocException(const char *file,
    const unsigned int line, size_t size) throw()
    : TraceableException(file, line, "Bad Allocation"), size(size),
      size_known(true) {
  // do nothing
}

BadAllocException::~BadAllocException() throw() {
  // do nothing
}

const char *BadAllocException::what() const throw() {
  if (!this->size_known) {
    return TraceableException::what();
  }
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << "\tcaused by request for "
      << this->size << " bytes\n";
  return ss.str().c_str();
}

}
