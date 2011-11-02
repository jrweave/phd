#include "ptr/APtr.h"

#include <new>

namespace ptr {

using namespace std;

template<typename arr_type>
APtr<arr_type>::APtr() throw(BadAllocException)
    : DPtr<arr_type>() {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(arr_type *p) throw(BadAllocException)
    : DPtr<arr_type>(p, 0) {
  this->size_known = false;
}

template<typename arr_type>
APtr<arr_type>::APtr(arr_type *p, size_t num) throw()
    : DPtr<arr_type>(p, num) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(size_t num) throw(BadAllocException)
    : DPtr<arr_type>((arr_type*)NULL, num) {
  try {
    this->p = new arr_type[num];
  } catch (bad_alloc &ba) {
    // num*sizeof(arr_type) may be a low estimate since
    // we're creating an array with new and not malloc
    THROW(BadAllocException, num*sizeof(arr_type));
  }
}

template<typename arr_type>
APtr<arr_type>::APtr(const APtr<arr_type> &aptr) throw()
    : DPtr<arr_type>(&aptr) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(const APtr<arr_type> *aptr) throw()
    : DPtr<arr_type>(aptr) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::~APtr() throw() {
  this->destruct();
}

template<typename arr_type>
void APtr<arr_type>::destroy() throw() {
  if (this->dptr() != NULL) {
    delete[] this->dptr();
  }
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
APtr<arr_type> &APtr<arr_type>::operator=(arr_type *p)
    throw(BadAllocException) {
  DPtr<arr_type>::operator=(p);
  return *this;
}

}
