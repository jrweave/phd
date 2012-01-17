#include "sys/char.h"

#include <cctype>
#include <cstring>
#include "sys/sys.h"

#if SYSTEM == SYS_DEFAULT
inline uint8_t to_ascii(char c) {
  return (uint8_t) c;
}

inline char to_lchar(uint8_t c) {
  return (char) c;
}

inline uint8_t *ascii_strcpy(uint8_t *ascii, const char *cstr) {
  return (uint8_t *)memcpy(ascii, cstr, strlen(cstr));
}

inline int ascii_strncmp(const uint8_t *ascii, const char *cstr,
    const size_t n) {
  return strncmp((const char *)ascii, cstr, n);
}
#else
#error Lacking definition of some char.h functions for specified system.
#endif

inline bool is_alnum(uint32_t c) {
  return is_digit(c) || is_alpha(c);
}

inline bool is_alpha(uint32_t c) {
  return is_lower(c) || is_upper(c);
}

inline bool is_cntrl(uint32_t c) {
  return c <= UINT32_C(0x1F) || c == UINT32_C(0x7F);
}

inline bool is_digit(uint32_t c) {
  return UINT32_C(0x30) <= c && c <= UINT32_C(0x39);
}

inline bool is_graph(uint32_t c) {
  return UINT32_C(0x21) <= c && c <= UINT32_C(0x7E);
}

inline bool is_lower(uint32_t c) {
  return UINT32_C(0x61) <= c && c <= UINT32_C(0x7A);
}

inline bool is_print(uint32_t c) {
  return UINT32_C(0x20) <= c && c <= UINT32_C(0x7E);
}

inline bool is_punct(uint32_t c) {
  return (UINT32_C(0x21) <= c && c <= UINT32_C(0x2F)) ||
         (UINT32_C(0x3A) <= c && c <= UINT32_C(0x40)) ||
         (UINT32_C(0x5B) <= c && c <= UINT32_C(0x60)) ||
         (UINT32_C(0x7B) <= c && c <= UINT32_C(0x7E));
}

inline bool is_space(uint32_t c) {
  return c == UINT32_C(0x20) ||
         (UINT32_C(0x09) <= c && c <= UINT32_C(0x0D));
}

inline bool is_upper(uint32_t c) {
  return UINT32_C(0x41) <= c && c <= UINT32_C(0x5A);
}

inline bool is_xdigit(uint32_t c) {
  return is_digit(c) ||
         (UINT32_C(0x41) <= c && c <= UINT32_C(0x46)) ||
         (UINT32_C(0x61) <= c && c <= UINT32_C(0x66));
}

inline uint32_t to_lower(uint32_t c) {
  return is_upper(c) ? c + UINT32_C(0x20) : c;
}

inline uint32_t to_upper(uint32_t c) {
  return is_lower(c) ? c - UINT32_C(0x20) : c;
}
