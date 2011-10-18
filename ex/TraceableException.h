#ifndef __TRACEABLEEXCEPTION_H__
#define __TRACEABLEEXCEPTION_H__

#include <exception>
#include <string>
#include <vector>

#define THROW(type, ...) \
  throw type(__FILE__, __LINE__, __VA_ARGS__) 

#define RETHROW(ex, ...) \
  throw ex.amendStackTrace(__FILE__, __LINE__, __VA_ARGS__)

#define THROWS(...) \
  throw(__VA_ARGS__) { \
    try

#define TRACE(...) \
    catch (TraceableException &_e) { \
      RETHROW(_e, __VA_ARGS__); \
    } \
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
  TraceableException(TraceableException *) throw();
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

#endif /* __TRACEABLEEXCEPTION_H__ */
