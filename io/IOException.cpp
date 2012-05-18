#include "io/IOException.h"

namespace io {

using namespace ex;
using namespace std;

IOException::IOException(const char *file, const unsigned int line)
    throw(const char *)
    : TraceableException(file, line) {
  // do nothing
}

IOException::IOException(const char *file, const unsigned int line,
    const char *message) throw(const char*)
    : TraceableException(file, line, message) {
  // do nothing
}

IOException::IOException(const IOException *ioe) throw()
    : TraceableException(ioe) {
  // do nothing
}

IOException::~IOException() throw() {
  // do nothing
}

}
