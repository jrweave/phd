#include <deque>
#include <string>
#include "io/BufferedInputStream.h"
#include "io/BufferedOutputStream.h"
#include "io/IFStream.h"
#include "io/InputStream.h"
#include "io/LZOInputStream.h"
#include "io/LZOOutputStream.h"
#include "io/OFStream.h"
#include "io/OutputStream.h"
#include "sys/endian.h"
#include "sys/ints.h"
#include "util/funcs.h"

using namespace io;
using namespace ptr;
using namespace std;
using namespace sys;
using namespace util;

struct cmdargs_t {
  string input;
  string output;
  string index;
  size_t block_size;
  size_t page_size;
  bool include_header;
  bool include_footer;
  bool include_checksum;
  bool decompress;
  bool allow_splitting;
  bool print_index;
} cmdargs = { string("-"), string("-"), string(""), 4096, 0, true, true, true, false, true, false };

bool parse_args(const int argc, char **argv) {
  int i;
  for (i = 1; i < argc; ++i) {
    if (string(argv[i]) == string("-d")) {
      cmdargs.decompress = true;
    } else if (string(argv[i]) == string("-o")) {
      if (cmdargs.output != string("-")) {
        cerr << "[ERROR] Only one output file can be specified." << endl;
        return false;
      }
      cmdargs.output = string(argv[++i]);
    } else if (string(argv[i]) == string("-nh")) {
      cmdargs.include_header = false;
    } else if (string(argv[i]) == string("-nf")) {
      cmdargs.include_footer = false;
    } else if (string(argv[i]) == string("-nc")) {
      cmdargs.include_checksum = false;
    } else if (string(argv[i]) == string("-ns")) {
      cmdargs.allow_splitting = false;
    } else if (string(argv[i]) == string("-i")) {
      if (cmdargs.index != string("")) {
        cerr << "[ERROR] Only one index file can be specified." << endl;
        return false;
      }
      cmdargs.index = string(argv[++i]);
    } else if (string(argv[i]) == string("-b")) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.block_size;
      if (cmdargs.block_size <= 0) {
        cerr << "[ERROR] Block size must be a positive number." << endl;
        return false;
      }
    } else if (string(argv[i]) == string("-p")) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.page_size;
    } else if (string(argv[i]) == string("--print-index")) {
      cmdargs.print_index = true;
    } else if (cmdargs.input != string("-")) {
      cerr << "[ERROR] Only one input file can be specified." << endl;
      return false;
    } else {
      cmdargs.input = string(argv[i]);
    }
  }
  return true;
}

void write_index(OutputStream *xs, deque<uint64_t> *index, DPtr<uint8_t> *nump) {
  if (is_big_endian()) {
    deque<uint64_t>::iterator it = index->begin();
    for (; it != index->end(); ++it) {
      if (!nump->alone()) {
        nump = nump->stand();
      }
      memcpy(nump->dptr(), &(*it), sizeof(uint64_t));
      xs->write(nump);
    }
    deque<uint64_t> swapper;
    index->swap(swapper);
  } else {
    deque<uint64_t>::iterator it = index->begin();
    for (; it != index->end(); ++it) {
      reverse_bytes(*it);
      if (!nump->alone()) {
        nump = nump->stand();
      }
      memcpy(nump->dptr(), &(*it), sizeof(uint64_t));
      xs->write(nump);
    }
    deque<uint64_t> swapper;
    index->swap(swapper);
  }
}

int print_index() {
  InputStream *is;
  if (cmdargs.input == string("-")) {
    NEW(is, IStream<istream>, cin);
  } else {
    NEW(is, IFStream, cmdargs.input.c_str());
  }
  NEW(is, BufferedInputStream, is, sizeof(uint64_t));
  uint64_t num;
  DPtr<uint8_t> *nump = is->read();
  if (is_big_endian()) {
    while (nump != NULL) {
      memcpy(&num, nump->dptr(), sizeof(uint64_t));
      nump->drop();
      cout << num << endl;
      nump = is->read();
    }
  } else {
    while (nump != NULL) {
      memcpy(&num, nump->dptr(), sizeof(uint64_t));
      nump->drop();
      reverse_bytes(num);
      cout << num << endl;
      nump = is->read();
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  if (!parse_args(argc, argv)) {
    return -1;
  }
  if (cmdargs.print_index) {
    return print_index();
  }
  DPtr<uint8_t> *nump = NULL;
  deque<uint64_t> *index = NULL;
  InputStream *is = NULL;
  OutputStream *os = NULL;
  OutputStream *xs = NULL;
  if (cmdargs.index != string("")) {
    if (cmdargs.index == string("-")) {
      NEW(xs, OStream<ostream>, cerr);
    } else {
      NEW(xs, OFStream, cmdargs.index.c_str());
    }
    if (cmdargs.page_size > 0) {
      NEW(xs, BufferedOutputStream, xs, cmdargs.page_size, true);
    }
    NEW(index, deque<uint64_t>);
    NEW(nump, MPtr<uint8_t>, sizeof(uint64_t));
  }
  if (cmdargs.input == string("-")) {
    NEW(is, IStream<istream>, cin);
  } else {
    NEW(is, IFStream, cmdargs.input.c_str());
  }
  if (cmdargs.decompress) {
    NEW(is, LZOInputStream, is, index);
  }
  if (cmdargs.output == string("-")) {
    NEW(os, OStream<ostream>, cout);
  } else {
    NEW(os, OFStream, cmdargs.output.c_str());
  }
  if (cmdargs.page_size > 0) {
    NEW(os, BufferedOutputStream, os, cmdargs.page_size, true);
  }
  if (!cmdargs.decompress) {
    NEW(os, LZOOutputStream, os, index, cmdargs.block_size,
        cmdargs.include_header, cmdargs.include_footer,
        cmdargs.include_checksum);
    NEW(os, BufferedOutputStream, os, cmdargs.block_size,
        cmdargs.allow_splitting);
  }
  DPtr<uint8_t> *readp = is->read();
  if (xs == NULL) {
    while (readp != NULL) {
      os->write(readp);
      readp->drop();
      readp = is->read();
    }
    is->close();
    os->close();
    DELETE(is);
    DELETE(os);
  } else {
    while (readp != NULL) {
      os->write(readp);
      readp->drop();
      if ((index->size() << 3) >= cmdargs.page_size) {
        write_index(xs, index, nump);
      }
      readp = is->read();
    }
    is->close();
    os->close();
    write_index(xs, index, nump);
    nump->drop();
    xs->close();
    DELETE(is);
    DELETE(os);
    DELETE(xs);
  }
}
