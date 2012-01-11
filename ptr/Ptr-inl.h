#include "ptr/Ptr.h"

namespace ptr {

using namespace std;

inline
Ptr::Ptr(const Ptr &ptr) throw()
    : p(ptr.p), local_refs(1), global_refs(ptr.global_refs) {
  (*(this->global_refs))++;
}

inline
Ptr::Ptr(const Ptr *ptr) throw()
    : p(ptr->p), local_refs(1), global_refs(ptr->global_refs) {
  (*(this->global_refs))++;
}

inline
Ptr::~Ptr() throw() {
  this->destruct();
}

inline
uint32_t Ptr::localRefs() const throw() {
  return this->local_refs;
}

inline
uint32_t Ptr::globalRefs() const throw() {
  return *(this->global_refs);
}

inline
void *Ptr::ptr() const throw() {
  return this->p;
}

inline
void Ptr::hold() throw() {
  this->local_refs++;
  (*(this->global_refs))++;
}

inline
bool Ptr::alone() const throw() {
  return *(this->global_refs) == 1;
}

inline
void Ptr::destroy() throw() {
  // This class doesn't actually destroy anything.
  // Subclasses should override this method with something meaningful.
}

inline
Ptr &Ptr::operator=(const Ptr &rhs) throw() {
  return *this = &rhs;
}

}
