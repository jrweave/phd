#include <sstream>

#include <cstring>
#include "ex/TraceableException.h"

namespace ex {

using namespace std;

TraceableException::TraceableException(const char *file,
    const unsigned int line) throw(const char *)
    : file(file), line(line) {
  if (file == NULL) {
    throw file;
  }
  memset(this->message, 0, 128);
}

TraceableException::TraceableException(const char *file,
    const unsigned int line, const char *message) throw(const char *) 
    : file(file), line(line) {
  if (file == NULL) {
    throw file;
  }
  memset(this->message, 0, 128);
  if (message != NULL) {
    strncpy(this->message, message, 127);
  }
}

TraceableException::TraceableException(const TraceableException *ex) throw ()
    : file(ex->file), line(ex->line) {
  memset(this->message, 0, 128);
  if (ex->message != NULL) {
    strncpy(this->message, ex->message, 127);
  }
  this->stack_trace.insert(this->stack_trace.begin(),
      ex->stack_trace.begin(), ex->stack_trace.end());
}

TraceableException::~TraceableException() throw() {
  // do nothing
}

const char *TraceableException::getFile() const throw() {
  return this->file;
}

unsigned int TraceableException::getLine() const throw() {
  return this->line;
}

const char *TraceableException::getMessage() const throw() {
  return this->message;
}

TraceableException &TraceableException::amendStackTrace(const char *file,
    const unsigned int line) throw(const char *) {
  return this->amendStackTrace(file, line, NULL);
}

TraceableException &TraceableException::amendStackTrace(const char *file,
    const unsigned int line, const char *message) throw(const char *) {
  if (file == NULL) {
    throw file;
  }
  stringstream ss (stringstream::in | stringstream::out);
  ss << file << ":" << line;
  if (message != NULL) {
    ss << ": " << message;
  }
  this->stack_trace.push_back(ss.str());
  return *this;
}

const char *TraceableException::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << this->file << ":" << line;
  if (this->message != NULL) {
    ss << ": " << this->message;
  }
  ss << "\n";
  vector<string>::const_iterator it;
  for (it = this->stack_trace.begin(); it != this->stack_trace.end(); it++) {
    ss << "\t\t" << *it << "\n";
  }
  return ss.str().c_str();
}

}

std::ostream &operator<<(std::ostream &stream, ex::TraceableException *ex) {
  return stream << ex->what();
}
