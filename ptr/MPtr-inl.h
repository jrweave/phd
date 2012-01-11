#include "ptr/MPtr.h"

#include <algorithm>
#include "ptr/alloc.h"

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
    : DPtr<ptr_type>((ptr_type*)NULL, num) {
  ptr_type *q;
  if (!alloc(q, num)) {
    THROW(BadAllocException, num*sizeof(ptr_type));
  }
  this->p = q;
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
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> *mptr, size_t offset) throw()
    : DPtr<ptr_type>(mptr, offset) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::MPtr(const MPtr<ptr_type> *mptr, size_t offset, size_t len)
    throw()
    : DPtr<ptr_type>(mptr, offset, len) {
  // do nothing
}

template<typename ptr_type>
MPtr<ptr_type>::~MPtr() throw() {
  this->destruct();
}

template<typename ptr_type>
void MPtr<ptr_type>::destroy() throw() {
  if (this->p != NULL) {
    dalloc(this->p);
  }
}

template<typename ptr_type>
DPtr<ptr_type> *MPtr<ptr_type>::sub(size_t offset) throw() {
  DPtr<ptr_type> *d;
  NEW(d, MPtr<ptr_type>, this, this->offset + offset);
  return d;
}

template<typename ptr_type>
DPtr<ptr_type> *MPtr<ptr_type>::sub(size_t offset, size_t len) throw() {
  DPtr<ptr_type> *d;
  NEW(d, MPtr<ptr_type>, this, this->offset + offset, len);
  return d;
}

template<typename ptr_type>
size_t MPtr<ptr_type>::sizeInBytes() const throw() {
  return this->size() * sizeof(ptr_type);
}

template<typename ptr_type>
DPtr<ptr_type> *MPtr<ptr_type>::stand() throw(BadAllocException) {
  if (this->alone()) {
    // TODO change to also ralloc underlying memory
    return this;
  }
  if (!this->sizeKnown()) {
    return NULL;
  }
  ptr_type *p;
  if (!alloc(p, this->size())) {
    THROW(BadAllocException, this->size() * sizeof(ptr_type));
  }
  copy(this->dptr(), this->dptr() + this->size(), p);
  if (this->localRefs() > 1) {
    DPtr<ptr_type> *d;
    NEW(d, MPtr<ptr_type>, p, this->size());
    this->drop();
    return d;
  }
  DPtr<ptr_type>::reset(p, true, this->size());
  return this;
}

template<typename ptr_type>
bool MPtr<ptr_type>::standable() const throw() {
  return this->alone() || this->sizeKnown();
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
