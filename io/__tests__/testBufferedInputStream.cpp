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

#include "test/unit.h"
#include "io/BufferedInputStream.h"

#include "io/IFStream.h"

using namespace io;
using namespace ptr;
using namespace std;

bool test(const char *filename, const size_t buffer_size,
          const size_t num_buffers, const size_t remainder) {
  size_t nbufs = 0;
  size_t rem = 0;
  InputStream *is;
  NEW(is, IFStream, filename);
  NEW(is, BufferedInputStream, is, buffer_size);
  DPtr<uint8_t> *p = is->read();
  while (p != NULL && p->size() == buffer_size) {
    ++nbufs;
    p->drop();
    p = is->read();
  }
  if (p != NULL) {
    rem = p->size();
    p->drop();
  }
  is->close();
  DELETE(is);
  PROG(num_buffers == nbufs);
  PROG(remainder == rem);
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(test, "foaf.nt", 1024, 10, 893);
  FINAL;
}
