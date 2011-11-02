#ifndef __MPTR_H__
#define __MPTR_H__

#include <cstring>
#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename ptr_type>
class MPtr : public DPtr<ptr_type> {
protected:
  virtual void destroy() throw();
  MPtr(const MPtr<ptr_type> *mptr, size_t offset) throw();
  MPtr(const MPtr<ptr_type> *mptr, size_t offset, size_t len) throw();
public:
  MPtr() throw(BadAllocException);
  MPtr(ptr_type *p) throw(BadAllocException);
  MPtr(ptr_type *p, size_t num) throw(BadAllocException);
  MPtr(size_t num) throw(BadAllocException);
  MPtr(const MPtr<ptr_type> &mptr) throw();
  MPtr(const MPtr<ptr_type> *mptr) throw();
  virtual ~MPtr() throw();

  // Overridden Methods
  virtual DPtr<ptr_type> *sub(size_t offset) throw();
  virtual DPtr<ptr_type> *sub(size_t offset, size_t len) throw();

  // Final Methods
  size_t sizeInBytes() const throw();

  // Operators
  MPtr<ptr_type> &operator=(const MPtr<ptr_type> &rhs) throw();
  MPtr<ptr_type> &operator=(const MPtr<ptr_type> *rhs) throw();
  MPtr<ptr_type> &operator=(ptr_type *p) throw(BadAllocException);
};

}

#include "ptr/MPtr-inl.h"

#endif /* __MPTR_H__ */
