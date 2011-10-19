#include "ptr/MPtr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
MPtr<ptr_type>::MPtr(ptr_type *p) throw()
    : DPtr<ptr_type>(p), num(0) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(ptr_type *p, size_t num) throw()
    : DPtr<ptr_type>(p), num(num) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(size_t num) throw(BadAllocException)
    : DPtr<ptr_type>((ptr_type *)calloc(num, sizeof(ptr_type))), num(num) {
  if (this->ptr() == NULL && num != 0) {
    THROW(BadAllocException, num*sizeof(ptr_type));
  }
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> &mptr) throw()
    : DPtr<ptr_type>(&mptr), num(mptr.num) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> *mptr) throw()
    : DPtr<ptr_type>(mptr), num(mptr->num) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::~MPtr() throw() {
  // do nothing
}

template<typename ptr_type>
void MPtr<ptr_type>::destroy() throw() {
  if (this->ptr() != NULL) {
    free(this->ptr());
  }
}

template<typename ptr_type>
bool MPtr<ptr_type>::sizeKnown() const throw() {
  return this->num != 0 || this->ptr() == NULL;
}

template<typename ptr_type>
size_t MPtr<ptr_type>::size() const throw() {
  return this->num;
}

template<typename ptr_type>
size_t MPtr<ptr_type>::sizeInBytes() const throw() {
  return this->size() * sizeof(ptr_type);
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(const MPtr<ptr_type> &rhs) throw() {
  Ptr *l = this;
  const Ptr *r = &rhs;
  *l = *r;
  return *this;
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(const MPtr<ptr_type> *rhs) throw() {
  Ptr *l = this;
  const Ptr *r = rhs;
  *l = r;
  return *this;
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(ptr_type *p)
    throw(BadAllocException) {
  Ptr::operator=((void*)p);
  return *this;
}

}
