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
#include "par/MPIPartialFileInputStream.h"

#include <cstdlib>
#include <fstream>
#include "sys/char.h"

using namespace ex;
using namespace par;
using namespace ptr;
using namespace std;

#define TEST_PAGE_SIZE 1024

bool test(const char *inputfile, const MPI::Offset begin, const MPI::Offset end, const char *verifyfile) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int size = MPI::COMM_WORLD.Get_size();

  if (rank == 0) {
    cout << "Input: " << inputfile << endl;
    cout << "Verify: " << verifyfile << endl;
  }

  MPI::Offset chunksize = (end - begin) / size;
  MPI::Offset mybegin = begin + rank * chunksize;
  MPI::Offset myend = (rank == size - 1 ? end : mybegin + chunksize);
  // ^^^ lazily given last processor the last incomplete chunk

  ONEBYONE_START
  cerr << "Processor " << rank << " has portion [" << mybegin << ',' << myend << ')' << endl;
  ONEBYONE_END

  InputStream *is;
  NEW(is, MPIPartialFileInputStream, MPI::COMM_WORLD, inputfile, MPI::MODE_RDONLY, MPI::INFO_NULL, TEST_PAGE_SIZE, mybegin, myend);
  int i;
  bool passed = true;
  for (i = 0; i < rank; ++i) {
    MPI::COMM_WORLD.Barrier();
  }
  cerr << "Processor " << i << " verifying its portion." << endl;
  ifstream ifs;
  ifs.open(verifyfile);
  cerr << "Processor " << i << " skipping " << (mybegin - begin) << " bytes." << endl;
  MPI::Offset j;
  for (j = begin; j < mybegin; ++j) {
    ifs.get();
  }
  DPtr<uint8_t> *p = is->read();
  while (p != NULL) {
    mybegin += p->size();
    const uint8_t *mark = p->dptr();
    const uint8_t *end = mark + p->size();
    for (; mark != end; ++mark) {
      if (!ifs.good()) {
        if (ifs.fail()) {
          cerr << "Processor " << i << " found that ifs failed." << endl;
        } else if (ifs.bad()) {
          cerr << "Processor " << i << " found that ifs is bad." << endl;
        } else if (ifs.eof()) {
          cerr << "Processor " << i << " found that ifs reached eof." << endl;
        }
        passed = false;
        break;
      }
      int c = ifs.get();
      if (*mark != c) {
        cerr << "Processor " << i << " found that byte " << (mybegin + (end - mark)) << " doesn't match (" << (char)*mark << " != " << (char) c << ')' << endl;
        passed = false;
        break;
      }
    }
    p->drop();
    if (!passed) {
      break;
    }
    p = is->read();
  }
  cerr << "Processor " << i << " verified that its portion " << (passed ? "passed." : "FAILED.") << endl;
  ifs.close();
  for (++i; i < size; ++i) {
    MPI::COMM_WORLD.Barrier();
  }
  is->close();
  DELETE(is);
  PROG(passed);
  PROG(mybegin == myend);
  PASS;
}

int main(int argc, char **argv) {
  INIT(argc, argv);
  int i;
  for (i = 1; i < argc; i += 2) {
    TEST(test, argv[i], 1000, 11000, argv[i+1]);
  }
  FINAL;
}
