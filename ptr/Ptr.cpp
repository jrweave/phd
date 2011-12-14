#include "ptr/Ptr.h"

#include <cstdlib>
#include <iostream>
#include "ptr/alloc.h"

namespace ptr {

using namespace std;

Ptr::Ptr() throw(BadAllocException)
    : p(NULL), local_refs(1) {
  if (!alloc(this->global_refs, 1)) {
    THROW(BadAllocException, sizeof(uint32_t));
  }
  *(this->global_refs) = 1;
}

Ptr::Ptr(void *p) throw(BadAllocException)
    : p(p), local_refs(1) {
  if (!alloc(this->global_refs, 1)) {
    THROW(BadAllocException, sizeof(uint32_t));
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
  this->destruct();
}

void Ptr::destruct() throw() {
  if (this->global_refs != NULL) {
    (*(this->global_refs)) -= this->local_refs;
    this->local_refs = 0;
    if (*(this->global_refs) == 0) {
      this->destroy();
      this->p = NULL;
      dalloc(this->global_refs);
      this->global_refs = NULL;
    }
  }
}

void Ptr::reset(void *p) throw(BadAllocException) {
  uint32_t *global;
  if (!alloc(global, 1)) {
    THROW(BadAllocException, sizeof(uint32_t));
  }
  *global = this->local_refs;
  this->destruct();
  this->p = p;
  this->global_refs = global;
  this->local_refs = *global;
}

uint32_t Ptr::localRefs() const throw() {
  return this->local_refs;
}

uint32_t Ptr::globalRefs() const throw() {
  return *(this->global_refs);
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
      this->p = NULL;
      dalloc(this->global_refs);
      this->global_refs = NULL;
    }
    void *x = this;
    DELETE(this);
  }
}

bool Ptr::alone() const throw() {
  return *(this->global_refs) == 1;
}

void Ptr::destroy() throw() {
  // This class doesn't actually destroy anything.
  // Subclasses should override this method with something meaningful.
}

Ptr &Ptr::operator=(const Ptr &rhs) throw() {
  return *this = &rhs;
}

Ptr &Ptr::operator=(const Ptr *rhs) throw() {
  if (this == rhs) {
    return *this;
  }
  (*(this->global_refs)) -= this->local_refs;
  // destroy old pointer if no one else refers to it.
  if (*(this->global_refs) == 0) {
    this->destroy();
    dalloc(this->global_refs);
    this->global_refs = NULL; // sanity
  }
  this->p = rhs->p;
  this->global_refs = rhs->global_refs;
  (*(this->global_refs)) += this->local_refs;
  return *this;
}

Ptr &Ptr::operator=(void *p) throw(BadAllocException) {
  (*(this->global_refs)) -= this->local_refs;
  if (*(this->global_refs) == 0) {
    if (this->p != p) {
      this->destroy();
    }
  } else {
    if (!alloc(this->global_refs, 1)) {
      THROW(BadAllocException, sizeof(uint32_t));
    }
  }
  this->p = p;
  (*(this->global_refs)) = this->local_refs;
  return *this;
}

Ptr *Ptr::thisAddr() throw() {
  return this;
}

}
