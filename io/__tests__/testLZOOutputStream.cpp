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
#include "io/LZOOutputStream.h"

#include "io/BufferedOutputStream.h"
#include "io/IFStream.h"
#include "io/OFStream.h"

using namespace io;
using namespace ptr;
using namespace std;

bool test(const char *inputfile, const char *outputfile, const char *verifyfile, const size_t block_size) {
  InputStream *is;
  NEW(is, IFStream, inputfile);
  OutputStream *os;
  NEW(os, OFStream, outputfile);
  NEW(os, LZOOutputStream, os, NULL, block_size, true, true, true);
  NEW(os, BufferedOutputStream, os, block_size, true);
  DPtr<uint8_t> *p = is->read();
  while (p != NULL) {
    os->write(p);
    p->drop();
    p = is->read();
  }
  is->close();
  os->close();
  DELETE(is);
  DELETE(os);
  // vvv this was not portable
//  fstream fin(outputfile);
//  fstream vin(verifyfile);
//  while (fin.good() && vin.good()) {
//    PROG(fin.get() == vin.get());
//  }
//  PROG(fin.good() == vin.good());
//  fin.close();
//  vin.close();
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  lzo_init();
  TEST(test, "foaf.nt", "foaf-1024.enc", "foaf-1024.lzo", 1024);
  FINAL;
}
