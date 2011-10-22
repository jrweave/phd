#ifndef __UCS_LOOKUP_H__
#define __UCS_LOOKUP_H__

#include <cstring>
#include <stdint.h>

#ifndef UINT32_C(c)
#define UINT32_C(c) ((unsigned long) c)
#endif /* UINT32_C */

namespace ucs {

using namespace std;

#include "ucs/ucs_arrays.h";

const ucs_char_data *ucs_lookup(const uint32_t codepoint);

const ucs_char_data *ucs_lookup_opt(const uint32_t codepoint);

}

#endif /* __UCS_LOOKUP_H__ */
