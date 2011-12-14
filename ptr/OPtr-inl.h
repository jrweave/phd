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
OPtr<obj_type>::OPtr(const OPtr<obj_type> *optr, size_t offset) throw()
    : DPtr<obj_type>(optr, offset) {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::OPtr(const OPtr<obj_type> *optr, size_t offset, size_t len)
    throw()
    : DPtr<obj_type>(optr, offset, len) {
  // do nothing
}

template<typename obj_type>
OPtr<obj_type>::~OPtr() throw() {
  this->destruct();
}

template<typename obj_type>
void OPtr<obj_type>::destroy() throw() {
  if (this->p != NULL) {
    DELETE((obj_type *)this->p);
  }
}

template<>
void OPtr<Ptr>::destroy() throw() {
  if (this->p != NULL) {
    ((Ptr *)this->p)->drop();
  }
}

template<typename obj_type>
DPtr<obj_type> *OPtr<obj_type>::sub(size_t offset) throw() {
  DPtr<obj_type> *d;
  NEW(d, OPtr<obj_type>, this, this->offset + offset);
  return d;
}

template<typename obj_type>
DPtr<obj_type> *OPtr<obj_type>::sub(size_t offset, size_t len) throw() {
  DPtr<obj_type> *d;
  NEW(d, OPtr<obj_type>, this, this->offset + offset, len);
  return d;
}

template<typename obj_type>
DPtr<obj_type> *OPtr<obj_type>::stand() throw(BadAllocException) {
  if (this->alone()) {
    return this;
  }
  // Making a copy of the underlying object depends on a copy constructor
  // or assignment operator.  Since we do not wish to restrict the
  // usefulness of this class to such objects, we simply return NULL.
  // In the future, it is conceivable to subclass OPtr to something
  // that will use the copy constructor or assignment operator.
  return NULL;
}

template<typename obj_type>
bool OPtr<obj_type>::standable() const throw() {
  return this->alone();
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
