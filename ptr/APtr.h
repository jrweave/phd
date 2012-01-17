#ifndef __PTR__APTR_H__
#define __PTR__APTR_H__

#include <cstring>
#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename arr_type>
class APtr : public DPtr<arr_type> {
protected:
  size_t actual_num;
  virtual void destroy() throw();
  APtr(const APtr<arr_type> *aptr, size_t offset) throw();
  APtr(const APtr<arr_type> *aptr, size_t offset, size_t len) throw();
public:
  APtr() throw(BadAllocException);
  APtr(arr_type *p) throw(BadAllocException);
  APtr(arr_type *p, size_t num) throw();
  APtr(size_t num) throw(BadAllocException);
  APtr(const APtr<arr_type> &mptr) throw();
  APtr(const APtr<arr_type> *mptr) throw();
  virtual ~APtr() throw();

  // Overridden Methods
  virtual DPtr<arr_type> *sub(size_t offset) throw();
  virtual DPtr<arr_type> *sub(size_t offset, size_t len) throw();
  virtual DPtr<arr_type> *stand() throw(BadAllocException);
  virtual bool standable() const throw();

  // Operators
  APtr<arr_type> &operator=(const APtr<arr_type> &rhs) throw();
  APtr<arr_type> &operator=(const APtr<arr_type> *rhs) throw();
  APtr<arr_type> &operator=(arr_type *p) throw(BadAllocException);
};

}

#include "ptr/APtr-inl.h"

#endif /* __PTR__APTR_H__ */
