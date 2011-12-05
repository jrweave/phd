#ifndef __IRI__MALFORMEDIRIREFEXCEPTION_H__
#define __IRI__MALFORMEDIRIREFEXCEPTION_H__

#include <cstring>
#include "ex/TraceableException.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

namespace iri {

using namespace ex;
using namespace ptr;
using namespace std;
using namespace ucs;

class MalformedIRIRefException : public TraceableException {
public:
  MalformedIRIRefException(const char *file, const unsigned int line) throw();
  MalformedIRIRefException(const char *file, const unsigned int line, UCSIter *mal)
      throw();
  virtual ~MalformedIRIRefException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __IRI__MALFORMEDIRIREFEXCEPTION_H__ */
