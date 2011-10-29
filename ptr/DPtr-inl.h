#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
DPtr<ptr_type>::DPtr() throw(BadAllocException)
    : Ptr(), num(0), size_known(false) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(ptr_type *p) throw(BadAllocException)
    : Ptr((void*)p), num(0), size_known(false) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(ptr_type *p, size_t size) throw(BadAllocException)
    : Ptr((void*)p), num(size), size_known(true) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(const DPtr<ptr_type> &dptr) throw()
    : Ptr(&dptr), num(dptr.size()), size_known(dptr.sizeKnown()) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(const DPtr<ptr_type> *dptr) throw()
    : Ptr(dptr), num(dptr->size()), size_known(dptr->sizeKnown()) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::~DPtr() throw() {
  this->destruct();
}

template<typename ptr_type>
ptr_type *DPtr<ptr_type>::dptr() const throw() {
  return (ptr_type *)this->ptr();
}

template<typename ptr_type>
bool DPtr<ptr_type>::sizeKnown() const throw() {
  return this->size_known;
}

template<typename ptr_type>
size_t DPtr<ptr_type>::size() const throw() {
  return this->num;
}

template<typename ptr_type>
DPtr<ptr_type> &DPtr<ptr_type>::operator=(const DPtr<ptr_type> &rhs) throw() {
  Ptr *l = this;
  const Ptr *r = &rhs;
  *l = *r;
  return *this;
}

template<typename ptr_type>
DPtr<ptr_type> &DPtr<ptr_type>::operator=(const DPtr<ptr_type> *rhs) throw() {
  Ptr *l = this;
  const Ptr *r = rhs;
  *l = r;
  return *this;
}

template<typename ptr_type>
DPtr<ptr_type> &DPtr<ptr_type>::operator=(ptr_type *p)
    throw(BadAllocException) {
  Ptr::operator=((void*)p);
  return *this;
}

template<typename ptr_type>
ptr_type &DPtr<ptr_type>::operator*() const throw() {
  return *(this->dptr());
}

template<typename ptr_type>
ptr_type &DPtr<ptr_type>::operator[](const size_t i) const throw() {
  return (this->dptr())[i];
}

template<typename ptr_type>
ptr_type *DPtr<ptr_type>::operator->() const throw() {
  return this->dptr();
}

}
