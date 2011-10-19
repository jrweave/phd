#include "ptr/Ptr.h"

#include <cstdlib>

namespace ptr {

using namespace std;

Ptr::Ptr(void *p) throw(BadAllocException)
    : p(p), local_refs(1) {
  this->global_refs = (unsigned int *)malloc(sizeof(unsigned int));
  if (this->global_refs == NULL) {
    THROW(BadAllocException, sizeof(unsigned int));
  }
  *(this->global_refs) = 1;
}

Ptr::Ptr(const Ptr &ptr) throw()
    : p(ptr.p), local_refs(1), global_refs(ptr.global_refs) {
  (*(this->global_refs))++;
}

Ptr::Ptr(const Ptr *ptr) throw()
    : p(ptr->p), local_refs(1), global_refs(ptr->global_refs) {
  (*(this->global_refs))++;
}

Ptr::~Ptr() throw() {
  if (this->global_refs != NULL) {
    (*(this->global_refs)) -= this->local_refs;
    if (*(this->global_refs) == 0) {
      free(this->global_refs);
    }
  }
}

void *Ptr::ptr() const throw() {
  return this->p;
}

void Ptr::hold() throw() {
  this->local_refs++;
  (*(this->global_refs))++;
}

void Ptr::drop() throw() {
  this->local_refs--;
  (*(this->global_refs))--;
  if (this->local_refs == 0) {
    if (*(this->global_refs) == 0) {
      this->destroy();
      free(this->global_refs);
      this->global_refs = NULL;
    }
    delete this;
  }
}

void Ptr::destroy() throw() {
  // This class doesn't actually destroy anything.
  // Subclasses should override this method with something meaningful.
}

Ptr &Ptr::operator=(const Ptr &rhs) throw() {
  // destroy old pointer if no one else refers to it.
  if (this->local_refs == *(this->global_refs)) {
    this->destroy();
  }
  this->p = rhs.p;
  this->local_refs = 1;
  this->global_refs = rhs.global_refs;
  (*(this->global_refs))++;
  return *this;
}

Ptr &Ptr::operator=(const Ptr *rhs) throw() {
  return *this = *rhs;
}

Ptr &Ptr::operator=(void *p) throw(BadAllocException) {
  if (this->local_refs == *(this->global_refs)) {
    this->destroy();
  } else {
    this->global_refs = (unsigned int *)malloc(sizeof(unsigned int));
    if (this->global_refs == NULL) {
      THROW(BadAllocException, sizeof(unsigned int));
    }
  }
  this->p = p;
  this->local_refs = 1;
  (*(this->global_refs)) = 1;
}

}
