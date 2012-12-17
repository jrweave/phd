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
