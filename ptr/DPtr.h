#ifndef __PTR__DPTR_H__
#define __PTR__DPTR_H__

#include "ptr/Ptr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
class DPtr : public Ptr {
protected:
  size_t num;
  size_t offset;
  bool size_known;
  void reset(ptr_type *p, bool sizeknown, size_t size)
      throw(BadAllocException);
  DPtr(const DPtr<ptr_type> *dptr, size_t offset) throw();
  DPtr(const DPtr<ptr_type> *dptr, size_t offset, size_t len) throw();
public:
  DPtr() throw(BadAllocException);
  DPtr(ptr_type *p) throw(BadAllocException);
  DPtr(ptr_type *p, size_t size) throw(BadAllocException);
  DPtr(const DPtr<ptr_type> &dptr) throw();
  DPtr(const DPtr<ptr_type> *dptr) throw();
  virtual ~DPtr() throw();

  // Final Methods
  ptr_type *dptr() const throw();
  bool sizeKnown() const throw();
  size_t size() const throw();

  // Virtual Methods
  virtual void *ptr() const throw();
  virtual DPtr<ptr_type> *sub(size_t offset) throw();
  virtual DPtr<ptr_type> *sub(size_t offset, size_t len) throw();
  virtual DPtr<ptr_type> *stand() throw(BadAllocException);
  virtual bool standable() const throw();

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

#endif /* __PTR__DPTR_H__ */
