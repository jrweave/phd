#include "ucs/ucs_lookup.h"

#include <iostream>

#define TEST(call, ...) \
  cerr << "TEST " #call "(" #__VA_ARGS__ ")\n"; \
  call(__VA_ARGS__)

#define PASS \
  cerr << "\nPASSED\n"; \
  return true

#define FAIL \
  cerr << " FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"; \
  return false

#define PROG(cond) \
  cerr << __LINE__; \
  if (!(cond)) { FAIL; } \
  cerr << ","

using namespace std;
using namespace ucs;

bool test0000() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x0));
  PROG(c != NULL);
  PROG(c->codepoint == UINT32_C(0x0));
  PROG(c->combining_class == 0);
  PROG(c->decomplen == 0);
  PASS;
}

bool testLetterA() {
  const ucs_char_data *c = ucs_lookup(UINT32_C('A'));
  PROG(c != NULL);
  cerr << "found codepoint " << c->codepoint << endl;
  PROG(c->codepoint == UINT32_C('A'));
  PROG(c->combining_class == 0);
  PROG(c->decomplen == 0);
  PASS;
}

bool testFDFA() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0xFDFA));
  PROG(c != NULL);
  PROG(c->codepoint == UINT32_C(0xFDFA));
  PROG(c->combining_class == 0);
  PROG(c->decomplen == 18);
  uint32_t decomp[] = {UINT32_C(0x0635), UINT32_C(0x0644), UINT32_C(0x0649), UINT32_C(0x0020), UINT32_C(0x0627), UINT32_C(0x0644), UINT32_C(0x0644), UINT32_C(0x0647), UINT32_C(0x0020), UINT32_C(0x0639), UINT32_C(0x0644), UINT32_C(0x064A), UINT32_C(0x0647), UINT32_C(0x0020), UINT32_C(0x0648), UINT32_C(0x0633), UINT32_C(0x0644), UINT32_C(0x0645), };
  size_t i;
  for (i = 0; i < c->decomplen; i++) {
    PROG(c->decomposition[i] == decomp[i]);
  }
  PASS;
}

bool test10FFFD() {
  const ucs_char_data *c = ucs_lookup_opt(UINT32_C(0x10FFFD));
  PROG(c != NULL);
  PROG(c->codepoint == UINT32_C(0x10FFFD));
  PROG(c->combining_class == 0);
  PROG(c->decomplen == 0);
  PASS;
}

bool test10FFFE() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x10FFFE));
  PROG(c == NULL);
  PASS;
}

bool test05C8() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x05C8));
  PROG(c == NULL);
  PASS;
}

bool test05C9() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x05C9));
  PROG(c == NULL);
  PASS;
}

bool test05CF() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x05CF));
  PROG(c == NULL);
  PASS;
}

bool test05D0() {
  const ucs_char_data *c = ucs_lookup(UINT32_C(0x05D0));
  PROG(c != NULL);
  PROG(c->codepoint == UINT32_C(0x05D0));
  PROG(c->combining_class == 0);
  PROG(c->decomplen == 0);
  PASS;
}

int main (int argc, char **argv) {
  TEST(test0000);
  TEST(testLetterA);
  TEST(testFDFA);
  TEST(test10FFFD);
  TEST(test05C8);
  TEST(test05C9);
  TEST(test05CF);
  TEST(test05D0);
}
