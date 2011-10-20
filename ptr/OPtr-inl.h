#include "ptr/OPtr.h"

#include "ptr/APtr.h"
#include "ptr/MPtr.h"

namespace ptr {

using namespace std;

template<typename obj_type>
OPtr<obj_type>::OPtr() throw(BadAllocException)
    : DPtr<obj_type>() {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::OPtr(obj_type *p) throw(BadAllocException)
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
  this->destruct();
}

template<typename obj_type>
void OPtr<obj_type>::destroy() throw() {
  if (this->ptr() != NULL) {
    delete this->dptr();
  }
}

template<>
void OPtr<Ptr>::destroy() throw() {
  if (this->ptr() != NULL) {
    this->dptr()->drop();
  }
}

template<typename obj_type>
bool OPtr<obj_type>::sizeKnown() const throw() {
  return true;
}

template<typename obj_type>
size_t OPtr<obj_type>::size() const throw() {
  return 1;
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
