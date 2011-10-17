template<class cause_type>
BaseException<cause_type>::BaseException(const char *file, unsigned int line,
    string msg) throw() {
  this->file_name = file;
  this->line_number = line;
  this->message = msg;
  this->use_cause = false;
}

template<class cause_type>
BaseException<cause_type>::BaseException(const char *file, unsigned int line,
    string msg, cause_type cause) throw() {
  this->file_name = file;
  this->line_number = line;
  this->message = msg;
  this->cause = cause;
  this->use_cause = true;
}

template<class cause_type>
BaseException<cause_type>::~BaseException() throw() {
  // do nothing
}

template<class cause_type>
BaseException<cause_type>&
    BaseException<cause_type>::operator=(const BaseException<cause_type> &rhs)
    throw() {
  this->file_name = rhs.getFileName();
  this->line_number = rhs.getLineNumber();
  this->message = rhs.getMessage();
  this->use_cause = rhs.hasCause();
  if(this->use_cause) {
    this->cause = rhs.getCause();
  }
  return *this;
}

template<class cause_type>
const char* BaseException<cause_type>::getFileName() const throw() {
  return this->file_name;
}

template<class cause_type>
unsigned int BaseException<cause_type>::getLineNumber() const throw() {
  return this->line_number;
}

template<class cause_type>
bool BaseException<cause_type>::hasCause() const throw() {
  return this->use_cause;
}

template<class cause_type>
cause_type BaseException<cause_type>::getCause()
    const throw(exception) {
  if(this->use_cause) {
    return this->cause;
  }
  throw *this;
}

template<class cause_type>
string BaseException<cause_type>::getMessage() const throw() {
  return this->message;
}

template<class cause_type>
vector<string>& BaseException<cause_type>::getStackTrace() throw() {
  return this->stack_trace;
}

template<class cause_type>
BaseException<cause_type>& BaseException<cause_type>::amendStackTrace(
    const char *file, unsigned int line, string msg) throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << file << ":" << line << ": " << msg;
  this->stack_trace.push_back(ss.str());
  return *this;
}

template<class cause_type>
const char* BaseException<cause_type>::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << this->file_name << ":" << this->line_number << ": " << this->message;
  vector<string>::const_iterator it;
  for(it = this->stack_trace.begin(); it != this->stack_trace.end(); it++) {
    ss << "\n\tto " << it->c_str();
  }
  if(this->use_cause) {
    ss << "Caused by " << this->cause;
  }
  return ss.str().c_str();
}
