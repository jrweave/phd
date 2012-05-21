#include "par/DistException.h"

namespace par {

using namespace ex;
using namespace std;

DistException::DistException(const char *file, const unsigned int line)
    throw(const char *)
    : TraceableException(file, line) {
  // do nothing
}

DistException::DistException(const char *file, const unsigned int line,
    const char *message) throw(const char*)
    : TraceableException(file, line, message) {
  // do nothing
}

DistException::DistException(const DistException *ioe) throw()
    : TraceableException(ioe) {
  // do nothing
}

DistException::~DistException() throw() {
  // do nothing
}

}
