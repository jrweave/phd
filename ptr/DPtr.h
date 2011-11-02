#ifndef __DPTR_H__
#define __DPTR_H__

#include "ptr/Ptr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
class DPtr : public Ptr {
protected:
  size_t num;
  size_t offset;
  bool size_known;
  DPtr(const DPtr<ptr_type> *dptr, size_t offset) throw();
  DPtr(const DPtr<ptr_type> *dptr, size_t offset, size_t len) throw();
public:
  DPtr() throw(BadAllocException);
  DPtr(ptr_type *p) throw(BadAllocException);
  DPtr(ptr_type *p, size_t size) throw(BadAllocException);
  DPtr(const DPtr<ptr_type> &dptr) throw();
  DPtr(const DPtr<ptr_type> *dptr) throw();
  virtual ~DPtr() throw();

  // Virtual Methods
  virtual void *ptr() const throw();
  virtual ptr_type *dptr() const throw();
  virtual bool sizeKnown() const throw();
  virtual size_t size() const throw();
  virtual DPtr<ptr_type> *sub(size_t offset) throw();
  virtual DPtr<ptr_type> *sub(size_t offset, size_t len) throw();

  // Operators
  DPtr<ptr_type> &operator=(const DPtr<ptr_type> &rhs) throw();
  DPtr<ptr_type> &operator=(const DPtr<ptr_type> *rhs) throw();
  DPtr<ptr_type> &operator=(ptr_type *p) throw(BadAllocException);
  virtual ptr_type &operator*() const throw();
  virtual ptr_type &operator[](const size_t i) const throw();
  virtual ptr_type *operator->() const throw();
};

}

#include "ptr/DPtr-inl.h"

#endif /* __DPTR_H__ */
