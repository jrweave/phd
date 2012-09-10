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

#include "par/__tests__/unit4mpi.h"
#include "par/MPIDistPtrFileOutputStream.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <set>
#include "io/IFStream.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIPacketDistributor.h"
#include "par/StringDistributor.h"
#include "rdf/NTriplesReader.h"
#include "rdf/RDFReader.h"
#include "rdf/RDFTriple.h"
#include "sys/char.h"

using namespace ex;
using namespace par;
using namespace ptr;
using namespace rdf;
using namespace std;

typedef multiset<RDFTriple, bool(*)(const RDFTriple&, const RDFTriple&)>
        TripleMultiset;

bool printAndEq(const RDFTriple &t1, const RDFTriple &t2) {
  bool eq = RDFTriple::cmpeq0(t1, t2);
  if (!eq)
    cerr << "NOT EQUAL ? ";
  DPtr<uint8_t> *str = t1.toUTF8String();
  const uint8_t *b = str->dptr();
  const uint8_t *e = b + str->size();
  for (; b != e; ++b) {
    cerr << to_lchar(*b);
  }
  str->drop();
  cerr << '\n';
  str = t2.toUTF8String();
  b = str->dptr();
  e = b + str->size();
  for (; b != e; ++b) {
    cerr << to_lchar(*b);
  }
  str->drop();
  cerr << endl;
  return eq;
}

void loadTriples(TripleMultiset &store, RDFReader *reader) {
  RDFTriple triple;
  while (reader->read(triple)) {
    store.insert(triple);
  }
}

void readNTriples(TripleMultiset &store, const char *filename) {
  IFStream *ifs;
  NEW(ifs, IFStream, filename);
  NTriplesReader ntr(ifs);
  loadTriples(store, &ntr);
  ntr.close();
}

bool test(char *filename, char *outfilename) {

  int rank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();

  TripleMultiset before(RDFTriple::cmplt0);
  TripleMultiset after(RDFTriple::cmplt0);
  deque<DPtr<uint8_t> *> lines;

  if (rank == 0) {
    readNTriples(before, filename);
  }

  MPIDelimFileInputStream *mis;
  NEW(mis, MPIDelimFileInputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL, 1024, to_ascii('\n'));

  DPtr<uint8_t> *input = mis->readDelimited();
  while (input != NULL) {
    lines.push_back(input);
    input = mis->readDelimited();
  }

  mis->close();
  DELETE(mis);

  uint8_t endline = to_ascii('\n');
  DPtr<uint8_t> endlptr(&endline, 1);
  MPIDistPtrFileOutputStream *mos;
  NEW(mos, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, outfilename,
      MPI::MODE_WRONLY | MPI::MODE_CREATE, MPI::INFO_NULL, 2048, true);

  deque<DPtr<uint8_t>*>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    DPtr<uint8_t> *line = *it;
    mos->write(line);
    line->drop();
    mos->write(&endlptr);
  }

  mos->close();
  DELETE(mos);

  MPI::COMM_WORLD.Barrier();

  if (rank == 0) {
    readNTriples(after, outfilename);
  }

  PROG(before.size() == after.size());
  PROG(equal(before.begin(), before.end(), after.begin(), RDFTriple::cmpeq0));
  PROG(equal(after.begin(), after.end(), before.begin(), RDFTriple::cmpeq0));

  PASS;
}

int main(int argc, char **argv) {
  INIT(argc, argv);
  TEST(test, argv[1], argv[2]);
  FINAL;
}
