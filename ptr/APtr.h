#ifndef __APTR_H__
#define __APTR_H__

#include <cstring>
#include "ptr/DPtr.h"

namespace ptr {

using namespace std;

template<typename arr_type>
class APtr : public DPtr<arr_type> {
private:
  size_t num;
  bool num_known;
protected:
  virtual void destroy() throw();
public:
  APtr() throw(BadAllocException);
  APtr(arr_type *p) throw(BadAllocException);
  APtr(arr_type *p, size_t num) throw();
  APtr(size_t num) throw(BadAllocException);
  APtr(const APtr<arr_type> &mptr) throw();
  APtr(const APtr<arr_type> *mptr) throw();
  virtual ~APtr() throw();

  // Inherited Methods
  virtual bool sizeKnown() const throw();
  virtual size_t size() const throw();

  // Operators
  APtr<arr_type> &operator=(const APtr<arr_type> &rhs) throw();
  APtr<arr_type> &operator=(const APtr<arr_type> *rhs) throw();
  APtr<arr_type> &operator=(arr_type *p) throw(BadAllocException);
};

}

#include "ptr/APtr-inl.h"

#endif /* __APTR_H__ */
