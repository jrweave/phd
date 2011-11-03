#ifndef __EX__BASEEXCEPTIONS_H__
#define __EX__BASEEXCEPTIONS_H__

#include "ex/TraceableException.h"

namespace ex {

using namespace std;

template<typename cause_type>
class BaseException : public TraceableException {
private:
  const cause_type cause;
public:
  BaseException(const char *file, const unsigned int line,
      const cause_type cause) throw();
  BaseException(const char *file, const unsigned int line,
      const cause_type cause, const char *message) throw();
  BaseException(const BaseException *) throw();
  virtual ~BaseException() throw();

  // Final Methods
  const cause_type getCause() const throw();

  // Inherited Methods
  virtual const char *what() const throw();
};

}

#include "ex/BaseException-inl.h"

#endif /* __EX__BASEEXCEPTIONS_H__ */
