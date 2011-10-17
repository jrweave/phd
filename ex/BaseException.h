#ifndef __BASEEXCEPTION_H__
#define __BASEEXCEPTION_H__

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define THROW(msg) \
  throw Exception(__FILE__, __LINE__, msg)

#define RETHROW(cause, msg) \
  throw cause.amendStackTrace(__FILE__, __LINE__, msg)

#define THROW_A(ex_type, ...) \
  throw ex_type(__FILE__, __LINE__, __VA_ARGS__)

#define THROW_TYPE(type, cause, msg) \
  throw BaseException<type>(__FILE__, __LINE__, cause, msg)

#define TRACE \
  catch(Exception &_cause) { \
    RETHROW(_cause, "(trace)"); \
  }

namespace ex {

using namespace std;

template <typename cause_type>
class BaseException : public exception {

private:
  vector<string> stack_trace;
  string message;
  unsigned int line_number;
  const char *file_name;
  cause_type cause;
  bool use_cause;

public:
  BaseException(const char *file, unsigned int line, string msg) throw();
  BaseException(const char *file, unsigned int line, string msg,
    cause_type cause) throw();
  virtual ~BaseException() throw();

  // Final Methods
  const char* getFileName() const throw();
  unsigned int getLineNumber() const throw();
  bool hasCause() const throw ();
  cause_type getCause() const throw(exception);
  string getMessage() const throw();
  vector<string>& getStackTrace() throw();
  BaseException<cause_type>& amendStackTrace(const char *file,
      unsigned int line, string msg) throw();

  // Inherited Methods
  virtual const char* what() const throw();

  // Operators
  BaseException<cause_type>& operator=(const BaseException<cause_type> &rhs)
      throw();
};

#include "ex/BaseException-inl.h"

typedef BaseException<exception> Exception;

}

std::ostream &operator<<(std::ostream &stream, std::exception ex) {
  return stream << "exception: " << ex.what();
}

template <typename cause_type>
std::ostream &operator<<(std::ostream &stream, ex::BaseException<cause_type> ex) {
  return stream << "exception: " << ex.what();
}

#endif /* __BASEEXCEPTION_H__ */
