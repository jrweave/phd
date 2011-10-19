#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
DPtr<ptr_type>::DPtr(ptr_type *p) throw()
    : Ptr((void*)p) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(const DPtr<ptr_type> &dptr) throw()
    : Ptr(&dptr) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::DPtr(const DPtr<ptr_type> *dptr) throw()
    : Ptr(dptr) {
  // do nothing
}

template<typename ptr_type>
DPtr<ptr_type>::~DPtr() throw() {
  // do nothing
  // Ptr::~Ptr will call destroy method,
  // which should be overridden by subclasses
}

template<typename ptr_type>
ptr_type *DPtr<ptr_type>::dptr() const throw() {
  return (ptr_type *)this->ptr();
}

template<typename ptr_type>
bool DPtr<ptr_type>::sizeKnown() const throw() {
  return false;
}

template<typename ptr_type>
size_t DPtr<ptr_type>::size() const throw() {
  return 0;
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
