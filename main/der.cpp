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

#include <deque>
#include <iomanip>
#include <string>
#include "io/BufferedInputStream.h"
#include "io/BufferedOutputStream.h"
#include "io/IFStream.h"
#include "io/InputStream.h"
#include "io/OFStream.h"
#include "io/OutputStream.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"
#include "rdf/RDFDictEncReader.h"
#include "rdf/RDFDictEncWriter.h"
#include "rdf/RDFDictionary.h"
#include "sys/endian.h"
#include "sys/ints.h"
#include "util/funcs.h"

#define NBYTES 8

using namespace io;
using namespace ptr;
using namespace rdf;
using namespace std;
using namespace sys;
using namespace util;

typedef RDFID<NBYTES> ID;

// HACK: This is a bit like cheating, but it works.
// This way, I can force some RDF terms to be encoded even if they
// do not appear in the data.
class CustomRDFEncoder {
public:
  static RDFDictionary<ID> dict;
  bool operator()(const RDFTerm &term, ID &id) {
    return dict.lookup(term, id);
  }
  bool operator()(const ID &id, RDFTerm &term) {
    if (dict.lookup(id, term)) {
      return true;
    }
    return dict.force(id, term);
  }
};

RDFDictionary<ID> CustomRDFEncoder::dict = RDFDictionary<ID>();

typedef CustomRDFEncoder ENC;

struct cmdargs_t {
  string input;
  string output;
  string index;
  size_t page_size;
  bool decompress;
  bool print_index;
} cmdargs = { string("-"), string("-"), string(""), 0, false, false };

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
    } else if (string(argv[i]) == string("-i")) {
      if (cmdargs.index != string("")) {
        cerr << "[ERROR] Only one index file can be specified." << endl;
        return false;
      }
      cmdargs.index = string(argv[++i]);
    } else if (string(argv[i]) == string("-p")) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.page_size;
    } else if (string(argv[i]) == string("--print-index")) {
      cmdargs.print_index = true;
    } else if (string(argv[i]) == string("--force")) {
      try {
        string termstr(argv[++i]);
        DPtr<uint8_t> *p;
        NEW(p, MPtr<uint8_t>, termstr.size());
        ascii_strcpy(p->dptr(), termstr.c_str());
        RDFTerm term = RDFTerm::parse(p);
        p->drop();
        CustomRDFEncoder::dict.encode(term);
      } catch (TraceableException &e) {
        cerr << "[ERROR] The following error occurred when trying to parse a value for --force.\n" << e.what() << endl;
        return false;
      }
    } else if (cmdargs.input != string("-")) {
      cerr << "[ERROR] Only one input file can be specified." << endl;
      return false;
    } else {
      cmdargs.input = string(argv[i]);
    }
  }
  if (cmdargs.decompress && cmdargs.index == string("")) {
    cerr << "[ERROR] Must specify a dictionary file with -i when decompressing." << endl;
    return -1;
  }
  return true;
}

int print_index() {
  InputStream *is;
  if (cmdargs.input == string("-")) {
    NEW(is, IStream<istream>, cin);
  } else {
    NEW(is, IFStream, cmdargs.input.c_str());
  }
  RDFDictionary<ID, ENC> *dict;
  NEW(dict, WHOLE(RDFDictionary<ID, ENC>));
  RDFDictEncReader<ID, ENC>::readDictionary(is, dict);
  is->close();
  DELETE(is);
  cout << setfill('0');
  RDFDictionary<ID, ENC>::const_iterator it = dict->begin();
  RDFDictionary<ID, ENC>::const_iterator end = dict->end();
  for (; it != end; ++it) {
    const uint8_t *p = it->first.ptr();
    const uint8_t *pend = p + ID::size();
    for (; p != pend; ++p) {
      cout << setw(2) << hex << (int)*p;
    }
    cout << setw(1) << dec << ' ' << it->second << endl;
  }
  DELETE(dict);
  RDFDictionary<ID>::const_iterator it2 = CustomRDFEncoder::dict.begin();
  RDFDictionary<ID>::const_iterator end2 = CustomRDFEncoder::dict.end();
  for (; it2 != end2; ++it2) {
    const uint8_t *p = it2->first.ptr();
    const uint8_t *pend = p + ID::size();
    cout << setw(2) << hex << (int)((*p) ^ UINT8_C(0x80));
    for (++p; p != pend; ++p) {
      cout << setw(2) << hex << (int)*p;
    }
    cout << setw(1) << dec << ' ' << it2->second << endl;
  }
  return 0;
}

int main(int argc, char **argv) {
  if (!parse_args(argc, argv)) {
    CustomRDFEncoder::dict.clear();
    ASSERTNPTR(0);
    return -1;
  }
  if (cmdargs.print_index) {
    int r = print_index();
    CustomRDFEncoder::dict.clear();
    ASSERTNPTR(0);
    return r;
  }
  RDFDictionary<ID, ENC> *dict;
  NEW(dict, WHOLE(RDFDictionary<ID, ENC>));
  InputStream *is = NULL;
  RDFReader *rr = NULL;
  OutputStream *os = NULL;
  RDFWriter *rw = NULL;
  if (cmdargs.decompress) {
    if (cmdargs.index == string("-")) {
      NEW(is, IStream<istream>, cin);
    } else {
      NEW(is, IFStream, cmdargs.index.c_str());
    }
    if (cmdargs.page_size > 0) {
      NEW(is, BufferedInputStream, is, cmdargs.page_size);
    }
    RDFDictEncReader<ID, ENC>::readDictionary(is, dict);
    is->close();
    DELETE(is);
    is = NULL;
  }
  if (cmdargs.input == string("-")) {
    NEW(is, IStream<istream>, cin);
  } else {
    NEW(is, IFStream, cmdargs.input.c_str());
  }
  if (cmdargs.page_size > 0) {
    NEW(is, BufferedInputStream, is, cmdargs.page_size);
  }
  if (cmdargs.decompress) {
    NEW(rr, WHOLE(RDFDictEncReader<ID, ENC>), is, dict, true, false);
  } else {
    NEW(rr, NTriplesReader, is);
  }
  if (cmdargs.output == string("-")) {
    NEW(os, OStream<ostream>, cout);
  } else {
    NEW(os, OFStream, cmdargs.output.c_str());
  }
  if (cmdargs.page_size > 0) {
    NEW(os, BufferedOutputStream, os, cmdargs.page_size, true);
  }
  if (cmdargs.decompress) {
    NEW(rw, NTriplesWriter, os);
  } else {
    NEW(rw, WHOLE(RDFDictEncWriter<ID, ENC>), os, dict, false);
  }
  RDFTriple triple;
  while (rr->read(triple)) {
    rw->write(triple);
  }
  triple = RDFTriple();
  rr->close();
  rw->close();
  DELETE(rr);
  DELETE(rw);
  if (!cmdargs.decompress && cmdargs.index != string("")) {
    if (cmdargs.index == string("-")) {
      NEW(os, OStream<ostream>, cerr);
    } else {
      NEW(os, OFStream, cmdargs.index.c_str());
    }
    if (cmdargs.page_size > 0) {
      NEW(os, BufferedOutputStream, os, cmdargs.page_size, true);
    }
    RDFDictEncWriter<ID, ENC>::writeDictionary(os, dict);
    ID bitflip(0);
    bitflip((ID::size() << 3) - 1, true);
    RDFDictEncWriter<ID>::writeDictionary(os, &CustomRDFEncoder::dict, bitflip);
    os->close();
    DELETE(os);
  }
  DELETE(dict);
  CustomRDFEncoder::dict.clear();
  ASSERTNPTR(0);
  return 0;
}
