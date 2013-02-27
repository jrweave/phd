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
#include "io/LZOInputStream.h"

#include "io/IFStream.h"
#include "io/OFStream.h"

using namespace io;
using namespace ptr;
using namespace std;

bool test(const char *inputfile, const char *outputfile, const char *verifyfile) {
  InputStream *is;
  NEW(is, IFStream, inputfile);
  NEW(is, LZOInputStream, is, NULL);
  OutputStream *os;
  NEW(os, OFStream, outputfile);
  DPtr<uint8_t> *p = is->read(1);
  while (p != NULL) {
    os->write(p);
    p->drop();
    p = is->read(1);
  }
  is->close();
  os->close();
  DELETE(is);
  DELETE(os);
  fstream fin(outputfile);
  fstream vin(verifyfile);
  while (fin.good() && vin.good()) {
    PROG(fin.get() == vin.get());
  }
  PROG(fin.good() == vin.good());
  fin.close();
  vin.close();
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(test, "foaf-1024.lzo", "foaf.dec", "foaf.nt");
  FINAL;
}
