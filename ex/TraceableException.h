#ifndef __TRACEABLEEXCEPTION_H__
#define __TRACEABLEEXCEPTION_H__

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#define THROWX(type) \
  throw type(__FILE__, __LINE__)

#define THROW(type, ...) \
  throw type(__FILE__, __LINE__, __VA_ARGS__) 

#define RETHROW(ex, ...) \
  ex.amendStackTrace(__FILE__, __LINE__, __VA_ARGS__); \
  throw ex

#define RETHROW_AS(type, e2) \
  THROW(type, e2.what())

#define THROWS(...) \
  throw(__VA_ARGS__) { \
    try

#define JUST_RETHROW(type, ...) \
  catch (type &_e) { \
    RETHROW(_e, __VA_ARGS__); \
  }

#define TRACE(type, ...) \
    JUST_RETHROW(type, __VA_ARGS__) \
  }

namespace ex {

using namespace std;

class TraceableException : public exception {
private:
  const char *file;
  const unsigned int line;
  const char *message;
  vector<string> stack_trace;
public:
  TraceableException(const char *file, const unsigned int line)
      throw(const char *);
  TraceableException(const char *file, const unsigned int line,
      const char *message) throw(const char *);
  TraceableException(const TraceableException *) throw();
  virtual ~TraceableException() throw();
  
  // Final Methods
  const char *getFile() const throw();
  unsigned int getLine() const throw();
  const char *getMessage() const throw();
  TraceableException &amendStackTrace(const char *file,
      const unsigned int line) throw(const char *);
  TraceableException &amendStackTrace(const char *file,
      const unsigned int line, const char *message) throw(const char *);

  // Virtual Methods
  virtual const char *what() const throw();
};

}

std::ostream &operator<<(std::ostream &stream, ex::TraceableException *ex);

#endif /* __TRACEABLEEXCEPTION_H__ */
