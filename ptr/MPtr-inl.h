#include "ptr/MPtr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
MPtr<ptr_type>::MPtr() throw(BadAllocException)
    : DPtr<ptr_type>() {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(ptr_type *p) throw(BadAllocException)
    : DPtr<ptr_type>(p) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(ptr_type *p, size_t num) throw(BadAllocException)
    : DPtr<ptr_type>(p, num) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(size_t num) throw(BadAllocException)
    : DPtr<ptr_type>((ptr_type *)calloc(num, sizeof(ptr_type)), num) {
  if (this->ptr() == NULL && num != 0) {
    THROW(BadAllocException, num*sizeof(ptr_type));
  }
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> &mptr) throw()
    : DPtr<ptr_type>(&mptr) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> *mptr) throw()
    : DPtr<ptr_type>(mptr) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::~MPtr() throw() {
  this->destruct();
}

template<typename ptr_type>
void MPtr<ptr_type>::destroy() throw() {
  if (this->ptr() != NULL) {
    free(this->ptr());
  }
}

template<typename ptr_type>
size_t MPtr<ptr_type>::sizeInBytes() const throw() {
  return this->size() * sizeof(ptr_type);
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(const MPtr<ptr_type> &rhs) throw() {
  DPtr<ptr_type> *l = this;
  const DPtr<ptr_type> *r = &rhs;
  *l = *r;
  return *this;
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(const MPtr<ptr_type> *rhs) throw() {
  DPtr<ptr_type> *l = this;
  const DPtr<ptr_type> *r = rhs;
  *l = r;
  return *this;
}

template<typename ptr_type>
MPtr<ptr_type> &MPtr<ptr_type>::operator=(ptr_type *p)
    throw(BadAllocException) {
  DPtr<ptr_type>::operator=(p);
  return *this;
}

}
