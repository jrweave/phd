#ifndef __OPTR_H__
#define __OPTR_H__

#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename obj_type>
class OPtr : public DPtr<obj_type> {
protected:
  virtual void destroy() throw();
  OPtr(const OPtr<obj_type> *optr, size_t offset) throw();
  OPtr(const OPtr<obj_type> *optr, size_t offset, size_t len) throw();
public:
  OPtr() throw(BadAllocException);
  OPtr(obj_type *p) throw(BadAllocException);
  OPtr(const OPtr<obj_type> &optr) throw();
  OPtr(const OPtr<obj_type> *optr) throw();
  virtual ~OPtr() throw();

  // Overridden Methods
  virtual DPtr<obj_type> *sub(size_t offset) throw();
  virtual DPtr<obj_type> *sub(size_t offset, size_t len) throw();

  // Operators
  OPtr<obj_type> &operator=(const OPtr<obj_type> &rhs) throw();
  OPtr<obj_type> &operator=(const OPtr<obj_type> *rhs) throw();
  OPtr<obj_type> &operator=(obj_type *p) throw(BadAllocException);
};

}

#include "ptr/OPtr-inl.h"

#endif /* __OPTR_H__ */
