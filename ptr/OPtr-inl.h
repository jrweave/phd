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
    : DPtr<obj_type>(p, 1) {
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
OPtr<obj_type> &OPtr<obj_type>::operator=(const OPtr<obj_type> &rhs) throw() {
  DPtr<obj_type> *l = this;
  const DPtr<obj_type> *r = &rhs;
  *l = *r;
  return *this;
}

template<typename obj_type>
OPtr<obj_type> &OPtr<obj_type>::operator=(const OPtr<obj_type> *rhs) throw() {
  DPtr<obj_type> *l = this;
  const DPtr<obj_type> *r = rhs;
  *l = r;
  return *this;
}

template<typename obj_type>
OPtr<obj_type> &OPtr<obj_type>::operator=(obj_type *p)
    throw(BadAllocException) {
  DPtr<obj_type>::operator=(p);
  return *this;
}

}
