#ifndef __SYS__CHAR_H__
#define __SYS__CHAR_H__

#include <cctype>
#include "sys/ints.h"

uint8_t to_ascii(char c);
char to_lchar(uint8_t c);
uint8_t *ascii_strcpy(uint8_t *ascii, const char *cstr);
int ascii_strncmp(const uint8_t *ascii, const char *cstr, const size_t n);
bool is_alnum(uint32_t c);
bool is_alpha(uint32_t c);
bool is_cntrl(uint32_t c);
bool is_digit(uint32_t c);
bool is_graph(uint32_t c);
bool is_lower(uint32_t c);
bool is_print(uint32_t c);
bool is_punct(uint32_t c);
bool is_space(uint32_t c);
bool is_upper(uint32_t c);
bool is_xdigit(uint32_t c);
uint32_t to_lower(uint32_t c);
uint32_t to_upper(uint32_t c);

#include "char-inl.h"

#endif /* __SYS__CHAR_H__ */
