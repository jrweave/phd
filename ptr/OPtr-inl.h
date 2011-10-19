#include "ptr/OPtr.h"

namespace ptr {

using namespace std;

template<typename obj_type>
OPtr<obj_type>::OPtr(obj_type *p) throw()
    : DPtr<obj_type>(p) {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::OPtr(const OPtr<obj_type> &optr) throw()
    : DPtr<obj_type>(&optr) {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::OPtr(const OPtr<obj_type> *optr) throw()
    : DPtr<obj_type>(optr) {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::~OPtr() throw() {
  // do nothing
  // Ptr::~Ptr will call destroy method
}

template<typename obj_type>
void OPtr<obj_type>::destroy() throw() {
  delete this->dptr();
}

template<typename obj_type>
OPtr<obj_type> &OPtr<obj_type>::operator=(const OPtr<obj_type> &rhs) throw() {
  Ptr *l = this;
  const Ptr *r = &rhs;
  *l = *r;
  return *this;
}

template<typename obj_type>
OPtr<obj_type> &OPtr<obj_type>::operator=(const OPtr<obj_type> *rhs) throw() {
  Ptr *l = this;
  const Ptr *r = rhs;
  *l = r;
  return *this;
}

template<typename obj_type>
OPtr<obj_type> &OPtr<obj_type>::operator=(obj_type *p)
    throw(BadAllocException) {
  Ptr::operator=((void*)p);
  return *this;
}

}
