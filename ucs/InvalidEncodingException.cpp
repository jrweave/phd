#include "ucs/InvalidEncodingException.h"

#include <sstream>
#include "sys/ints.h"

namespace ucs {

using namespace ex;
using namespace std;

InvalidEncodingException::InvalidEncodingException(const char *file,
    const unsigned int line, const char *message, const uint32_t encoded)
    throw()
    : TraceableException(file, line, message), encoded(encoded) { 
  // do nothing
}

InvalidEncodingException::~InvalidEncodingException() throw() {
  // do nothing
}

const char *InvalidEncodingException::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << "\tcaused by invalid encoding 0x" << hex << this->encoded << "\n";
  return ss.str().c_str();
}

}
