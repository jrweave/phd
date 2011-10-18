#include "ex/BaseException.h"

#include <sstream>

namespace ex {

using namespace std;

template<typename cause_type>
BaseException<cause_type>::BaseException(const char *file,
    const unsigned int line, const cause_type cause) throw()
    : TraceableException(file, line), cause(cause) {
  // do nothing
}

template<typename cause_type>
BaseException<cause_type>::BaseException(const char *file,
    const unsigned int line, const cause_type cause,
    const char *message) throw()
    : TraceableException(file, line, message), cause(cause) {
  // do nothing
}

template<typename cause_type>
BaseException<cause_type>::BaseException(const BaseException *ex) throw()
    : TraceableException(ex), cause(ex->cause) {
  // do nothing
}

template<typename cause_type>
BaseException<cause_type>::~BaseException() throw() {
  // do nothing
}

template<typename cause_type>
const cause_type BaseException<cause_type>::getCause() const throw() {
  return this->cause;
}

template<typename cause_type>
const char *BaseException<cause_type>::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << "\tcaused by " << this->cause << endl;
  return ss.str().c_str();
}

}
