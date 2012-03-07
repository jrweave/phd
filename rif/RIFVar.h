#ifndef __RIF__RIFVAR_H__
#define __RIF__RIFVAR_H__

#include "ex/BaseException.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"
#include "ucs/InvalidCodepointException.h"
#include "ucs/InvalidEncodingException.h"

namespace rif {

using namespace ex;
using namespace ptr;
using namespace std;
using namespace ucs;

template<typename iter>
bool isNCName(iter begin, iter end);

class RIFVar {
private:
  DPtr<uint8_t> *utf8name;
  bool normalized;
public:
  RIFVar() throw(); // empty string represented with NULL
  RIFVar(DPtr<uint8_t> *utf8name)
      throw(BaseException<void*>, SizeUnknownException,
            InvalidCodepointException, InvalidEncodingException);
  RIFVar(const RIFVar &copy) throw();
  ~RIFVar() throw();

  static int cmp(const RIFVar &var1, const RIFVar &var2) throw();
  static bool cmplt0(const RIFVar &var1, const RIFVar &var2) throw();
  static bool cmpeq0(const RIFVar &var1, const RIFVar &var2) throw();
  static RIFVar parse(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, InvalidCodepointException,
            InvalidEncodingException, BaseException<void*>,
            SizeUnknownException, TraceableException);
  static bool isNCNameStartChar(const uint32_t codepoint) throw();
  static bool isNCNameChar(const uint32_t codepoint) throw();

  bool equals(const RIFVar &rhs) const throw();
  RIFVar &normalize() throw(BadAllocException);
  DPtr<uint8_t> *getName() const throw();
  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);

  RIFVar &operator=(const RIFVar &rhs) throw();
};

}

#include "rif/RIFVar-inl.h"

#endif /* __RIF__RIFVAR_H__ */
