#include "ptr/APtr.h"

#include <algorithm>
#include <new>
#include "ptr/alloc.h"

namespace ptr {

using namespace std;

template<typename arr_type>
inline
APtr<arr_type>::APtr() throw(BadAllocException)
    : DPtr<arr_type>() {
  // do nothing
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(arr_type *p) throw(BadAllocException)
    : DPtr<arr_type>(p, 0) {
  this->size_known = false;
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(arr_type *p, size_t num) throw()
    : DPtr<arr_type>(p, num) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(size_t num) throw(BadAllocException)
    : DPtr<arr_type>((arr_type*)NULL, num) {
  try {
    NEW_ARRAY(this->p, arr_type, num);
  } catch (bad_alloc &ba) {
    // num*sizeof(arr_type) may be a low estimate since
    // we're creating an array with new and not malloc
    THROW(BadAllocException, num*sizeof(arr_type));
  }
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(const APtr<arr_type> &aptr) throw()
    : DPtr<arr_type>(&aptr) {
  // do nothing
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(const APtr<arr_type> *aptr) throw()
    : DPtr<arr_type>(aptr) {
  // do nothing
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(const APtr<arr_type> *aptr, size_t offset) throw()
    : DPtr<arr_type>(aptr, offset) {
  // do nothing
}

template<typename arr_type>
inline
APtr<arr_type>::APtr(const APtr<arr_type> *aptr, size_t offset, size_t len)
    throw()
    : DPtr<arr_type>(aptr, offset, len) {
  // do nothing
}

template<typename arr_type>
inline
APtr<arr_type>::~APtr() throw() {
  this->destruct();
}

template<typename arr_type>
inline
void APtr<arr_type>::destroy() throw() {
  if (this->p != NULL) {
    DELETE_ARRAY((arr_type *)this->p);
  }
}

template<typename arr_type>
inline
DPtr<arr_type> *APtr<arr_type>::sub(size_t offset) throw() {
  DPtr<arr_type> *d;
  NEW(d, APtr<arr_type>, this, this->offset + offset);
  return d;
}

template<typename arr_type>
inline
DPtr<arr_type> *APtr<arr_type>::sub(size_t offset, size_t len) throw() {
  DPtr<arr_type> *d;
  NEW(d, APtr<arr_type>, this, this->offset + offset, len);
  return d;
}

template<typename arr_type>
DPtr<arr_type> *APtr<arr_type>::stand() throw(BadAllocException) {
  if (this->alone()) {
    return this;
  }
  if (!this->sizeKnown()) {
    return NULL;
  }
  arr_type *arr = NULL;
  try {
    NEW_ARRAY(arr, arr_type, this->size());
  } RETHROW_BAD_ALLOC
  copy(this->dptr(), this->dptr() + this->size(), arr);
  if (this->localRefs() > 1) {
    DPtr<arr_type> *d;
    NEW(d, APtr<arr_type>, arr, this->size());
    this->drop();
    return d;
  }
  DPtr<arr_type>::reset(arr, true, this->size());
  return this;
}

template<typename arr_type>
inline
bool APtr<arr_type>::standable() const throw() {
  return this->alone() || this->sizeKnown();
}

template<typename arr_type>
APtr<arr_type> &APtr<arr_type>::operator=(const APtr<arr_type> &rhs) throw() {
  DPtr<arr_type> *l = this;
  const DPtr<arr_type> *r = &rhs;
  *l = *r;
  return *this;
}

template<typename arr_type>
APtr<arr_type> &APtr<arr_type>::operator=(const APtr<arr_type> *rhs) throw() {
  DPtr<arr_type> *l = this;
  const DPtr<arr_type> *r = rhs;
  *l = r;
  return *this;
}

template<typename arr_type>
inline
APtr<arr_type> &APtr<arr_type>::operator=(arr_type *p)
    throw(BadAllocException) {
  DPtr<arr_type>::operator=(p);
  return *this;
}

}
