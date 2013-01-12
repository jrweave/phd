/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#include "sys/endian.h"

#include "sys/ints.h"
#include "sys/sys.h"

namespace sys {

using namespace std;

// TODO check for common BYTE_ORDER macros

typedef union {
  uint32_t i;
  uint8_t c[4];
} __endian_check_t;

extern const __endian_check_t __endint;

inline
bool is_big_endian() {
#if SYSTEM
#if SYSTEM == SYS_CCNI_OPTERONS || \
    SYSTEM == SYS_MASTIFF
  return false;
#elif SYSTEM == SYS_BLUE_GENE_L || \
      SYSTEM == SYS_BLUE_GENE_P || \
      SYSTEM == SYS_BLUE_GENE_Q || \
      SYSTEM == SYS_CRAY_XMT    || \
      SYSTEM == SYS_CRAY_XMT_2
  return true;
#else
  return __endint.c[0] == 1;
#endif
#else
  return __endint.c[0] == 1;
#endif
}

inline
bool is_little_endian() {
#ifdef SYSTEM
#if SYSTEM == SYS_CCNI_OPTERONS || \
    SYSTEM == SYS_MASTIFF
  return true;
#elif SYSTEM == SYS_BLUE_GENE_L || \
      SYSTEM == SYS_BLUE_GENE_P || \
      SYSTEM == SYS_BLUE_GENE_Q || \
      SYSTEM == SYS_CRAY_XMT    || \
      SYSTEM == SYS_CRAY_XMT_2
  return false;
#else
  return __endint.c[0] == 4;
#endif
#else
  return __endint.c[0] == 4;
#endif
}

}
