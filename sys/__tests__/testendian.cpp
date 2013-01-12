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

#include "sys/sys.h"
#include "test/unit.h"

using namespace std;
using namespace sys;

bool testEndianess() {
  __endian_check_t check = { UINT32_C(0x01020304) };
  PROG(is_little_endian() || check.c[0] != 4);
  PROG(is_big_endian() || check.c[0] != 1);
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(testEndianess);
  FINAL;
}
