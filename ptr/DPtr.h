#ifndef __DPTR_H__
#define __DPTR_H__

#include "ptr/Ptr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
class DPtr : public Ptr {
public:
  DPtr(ptr_type *p) throw();
  DPtr(const DPtr<ptr_type> &dptr) throw();
  DPtr(const DPtr<ptr_type> *dptr) throw();
  virtual ~DPtr() throw();

  // Final Methods
  ptr_type *dptr() const throw();

  // Virtual Methods
  virtual bool sizeKnown() const throw();
  virtual size_t size() const throw();

  // Operators
  DPtr<ptr_type> &operator=(const DPtr<ptr_type> &rhs) throw();
  DPtr<ptr_type> &operator=(const DPtr<ptr_type> *rhs) throw();
  DPtr<ptr_type> &operator=(ptr_type *p) throw(BadAllocException);
  ptr_type &operator*() const throw();
  ptr_type &operator[](const size_t i) const throw();
  ptr_type *operator->() const throw();
};

}

#include "ptr/DPtr-inl.h"

#endif /* __DPTR_H__ */
