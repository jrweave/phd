#include "ucs/ucs_lookup.h"

#include <algorithm>
#include <iostream>`

namespace ucs {

using namespace std;

const ucs_char_data *ucs_lookup(const uint32_t codepoint) {
  const uint32_t *lb = lower_bound(UCS_RANGES,
      UCS_RANGES + UCS_RANGES_LEN, codepoint);
  cerr << "pointers " << UCS_RANGES << " " << lb << " " << UCS_RANGES + UCS_RANGES_LEN << endl;
  cerr << "*lb == " << *lb << endl;
  if (lb == UCS_RANGES + UCS_RANGES_LEN || *lb > codepoint) {
    lb--;
  }
  uint32_t offset = lb - UCS_RANGES;
  cerr << offset << endl;
  if ((offset & UINT32_C(1)) != UINT32_C(0)) {
    return NULL;
  }
  offset >>= 1;
  cerr << offset << endl;
  offset = UCS_RANGE_INDEX[offset];
  cerr << offset << endl;
  offset += (codepoint - *lb);
  cerr << offset << endl;
  return UCS_CHAR_DATA + offset;
}

const ucs_char_data *ucs_lookup_opt(const uint32_t codepoint) {
  if (codepoint < UCS_FIRST_BOUND) {
    return UCS_CHAR_DATA + codepoint;
  }
  return ucs_lookup(codepoint);
}

}
