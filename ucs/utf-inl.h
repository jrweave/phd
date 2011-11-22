#include "ucs/utf.h"

#include <algorithm>
#include "sys/endian.h"

namespace ucs {

using namespace ptr;
using namespace std;
using namespace sys;

template<class input_iter, class output_iter>
size_t utf8enc(input_iter begin, input_iter end, output_iter out)
    THROWS(InvalidEncodingException) {
  size_t total_len = 0;
  for (; begin != end; begin++) {
    uint8_t buf[4];
    size_t len = utf8len(*begin, buf);
    copy(buf, buf + len, out);
    out += len;
    total_len += len;
  }
  return total_len;
}
TRACE(InvalidEncodingException, "(trace)")

template<typename input_iter, typename output_iter>
size_t utf16enc(input_iter begin, input_iter end, const enum BOM bom,
    output_iter out) THROWS(InvalidEncodingException) {
  size_t total_len = 0;
  if (begin == end) {
    return 0;
  }
  if (bom != NONE) {
    if ((is_little_endian() && bom == BIG)
        || (is_big_endian() && bom == LITTLE)) {
      *out = UINT16_C(0xFFFE); // reverse
    } else {
      *out = UINT16_C(0xFEFF);
    }
    out++;
    total_len++;
  }
  for (; begin != end; begin++) {
    uint16_t buf[2];
    size_t len = utf16len(*begin, bom, buf);
    copy(buf, buf + len, out);
    out += len;
    total_len += len;
  }
  return total_len;
}
TRACE(InvalidEncodingException, "(trace)")

template<typename input_iter, typename output_iter>
size_t utf32enc(input_iter begin, input_iter end, const enum BOM bom,
    output_iter out) THROWS(InvalidEncodingException) {
  size_t total_len = 0;
  if (begin == end) {
    return 0;
  }
  if (bom != NONE) {
    if ((is_little_endian() && bom == BIG)
        || (is_big_endian() && bom == LITTLE)) {
      *out = UINT32_C(0xFFFE0000); // reverse
    } else {
      *out = UINT32_C(0x0000FEFF);
    }
    out++;
    total_len++;
  }
  for (; begin != end; begin++) {
    utf32len(*begin, bom, &(*out));
    out++;
    total_len++;
  }
  return total_len;
}
TRACE(InvalidEncodingException, "(trace)")

}
