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

#include <iostream>
#include <map>
#include <string>
#include "io/IFStream.h"
#include "io/OFStream.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"

using namespace io;
using namespace rdf;
using namespace std;

int main (int argc, char **argv) {
  map<string, size_t> errors;
  int i;
  NTriplesWriter *ntw = NULL;
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == '\0') {
      if (i < argc - 1) {
        ++i;
        OutputStream *ofs;
        cerr << "Writing to " << argv[i] << endl;
        NEW(ofs, OFStream, argv[i]);
        if (ntw != NULL) {
          ntw->close();
          DELETE(ntw);
        }
        NEW(ntw, NTriplesWriter, ofs);
      }
      continue;
    }
    cerr << "Normalizing " << argv[i] << endl;
    size_t numinput = 0;
    size_t numoutput = 0;
    size_t numnormalized = 0;
    size_t numerror = 0;
    InputStream *ifs;
    cerr << "Reading from " << argv[i] << endl;
    NEW(ifs, IFStream, argv[i]);
    NTriplesReader ntr(ifs);
    RDFTriple triple;
    for (;;) {
      try {
        if (!ntr.read(triple)) {
          ntr.close();
          break;
        }
        RDFTriple norm(triple);
        norm.normalize();
        if (!norm.equals(triple)) {
          //cerr << "Normalized: " << triple << "     --->     " << norm << endl;
          ++numnormalized;
        }
        if (ntw == NULL) {
          cout << norm;
        } else {
          ntw->write(norm);
        }
        ++numinput;
        ++numoutput;
      } catch (TraceableException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        cerr << "[ERROR] " << e.what() << endl;
        ++numinput;
        ++numerror;
        continue;
      }
    }
    cerr << "===== ERROR SUMMARY =====" << endl;
    map<string, size_t>::iterator it = errors.begin();
    for (; it != errors.end(); ++it) {
      cerr << "\t[" << it->second << "] " << it->first;
    }
    cerr << "===== SUMMARY =====" << endl;
    cerr << "Input triples: " << numinput << endl;
    cerr << "Non-normalized triples: " << numnormalized << endl;
    cerr << "Malformed triples: " << numerror << endl;
    cerr << "Output triples: " << numoutput << endl;
  }
  if (ntw != NULL) {
    ntw->close();
    DELETE(ntw);
  }
  ASSERTNPTR(0);
}
