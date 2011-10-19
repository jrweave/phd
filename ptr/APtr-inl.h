#include "ptr/APtr.h"

#include <new>

namespace ptr {

using namespace std;

template<typename arr_type>
APtr<arr_type>::APtr(arr_type *p) throw()
    : DPtr<arr_type>(p), num(0), num_known(false) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(arr_type *p, size_t num) throw()
    : DPtr<arr_type>(p), num(num), num_known(true) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(size_t num) throw(BadAllocException)
    : DPtr<arr_type>((arr_type*)NULL), num(num), num_known(true) {
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
    : DPtr<arr_type>(&aptr), num(aptr.num) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::APtr(const APtr<arr_type> *aptr) throw()
    : DPtr<arr_type>(aptr), num(aptr->num) {
  // do nothing
}

template<typename arr_type>
APtr<arr_type>::~APtr() throw() {
  if (!this->alreadyDestroyed()) {
    this->destroy();
  }
}

template<typename arr_type>
void APtr<arr_type>::destroy() throw() {
  if (this->dptr() != NULL) {
    delete[] this->dptr();
  }
}

template<typename arr_type>
bool APtr<arr_type>::sizeKnown() const throw() {
  return this->num_known;
}

template<typename arr_type>
size_t APtr<arr_type>::size() const throw() {
  return this->num;
}

template<typename arr_type>
APtr<arr_type> &APtr<arr_type>::operator=(const APtr<arr_type> &rhs) throw() {
  Ptr *l = this;
  const Ptr *r = &rhs;
  *l = *r;
  return *this;
}

template<typename arr_type>
APtr<arr_type> &APtr<arr_type>::operator=(const APtr<arr_type> *rhs) throw() {
  Ptr *l = this;
  const Ptr *r = rhs;
  *l = r;
  return *this;
}

template<typename arr_type>
APtr<arr_type> &APtr<arr_type>::operator=(arr_type *p)
    throw(BadAllocException) {
  Ptr::operator=((void*)p);
  return *this;
}

}
