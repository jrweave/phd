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
#include <mpi.h>
#include <string>
#include "io/BufferedInputStream.h"
#include "io/BufferedOutputStream.h"
#include "io/IFStream.h"
#include "io/InputStream.h"
#include "io/LZOInputStream.h"
#include "io/LZOOutputStream.h"
#include "io/OFStream.h"
#include "io/OutputStream.h"
#include "par/MPIDistPtrFileOutputStream.h"
#include "par/MPIPartialFileInputStream.h"
#include "sys/endian.h"
#include "sys/ints.h"
#include "util/funcs.h"

using namespace io;
using namespace par;
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
  bool single_input;
} cmdargs = { string("-"), string("-"), string(""), 4096, 4096, true, true, true, false, true, false, true };

bool parse_args(const int argc, char **argv) {
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  int i;
  for (i = 1; i < argc; ++i) {
    if (string(argv[i]) == string("-d")) {
      cmdargs.decompress = true;
    } else if (string(argv[i]) == string("-o")) {
      if (cmdargs.output != string("-")) {
        if (commrank == 0) {
          cerr << "[ERROR] Only one output file can be specified." << endl;
        }
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
        if (commrank == 0) {
          cerr << "[ERROR] Only one index file can be specified." << endl;
        }
        return false;
      }
      cmdargs.index = string(argv[++i]);
    } else if (string(argv[i]) == string("-b")) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.block_size;
      if (cmdargs.block_size <= 0) {
        if (commrank == 0) {
          cerr << "[ERROR] Block size must be a positive number." << endl;
        }
        return false;
      }
    } else if (string(argv[i]) == string("-p")) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.page_size;
      if (cmdargs.page_size <= 0) {
        if (commrank == 0) {
          cerr << "[ERROR] Page size must be a positive number." << endl;
        }
        return false;
      }
    } else if (string(argv[i]) == string("--print-index")) {
      cmdargs.print_index = true;
    } else if (cmdargs.input != string("-")) {
      if (commrank == 0) {
        cerr << "[ERROR] Only one input file can be specified." << endl;
      }
      return false;
    } else {
      cmdargs.input = string(argv[i]);
    }
  }
  if (cmdargs.print_index) {
    return true;
  }
  if (commsize > 1) {
    if (cmdargs.input == string("-")) {
      if (commrank == 0) {
        cerr << "[ERROR] Must specify input file (that is not stdin)." << endl;
      }
      return false;
    }
    if (cmdargs.output == string("-")) {
      if (cmdargs.decompress) {
        if (commrank == 0) {
          cerr << "[WARNING] Output going to stdout may not be in order." << endl;
        }
      } else {
        if (commrank == 0) {
          cerr << "[ERROR] Cannot compress to stdout when using multiple processes." << endl;
        }
        return false;
      }
    }
  }
  size_t hash = cmdargs.input.find('#');
  cmdargs.single_input = (hash == string::npos);
  if (cmdargs.single_input) {
    if (cmdargs.decompress && cmdargs.index == string("") && commsize > 1) {
      if (commrank == 0) {
        cerr << "[ERROR] Must specify an index file with -i to do parallel decompression on a single file (no '#' in file name)." << endl;
      }
      return false;
    }
  } else {
    stringstream ss (stringstream::in | stringstream::out);
    ss << cmdargs.input.substr(0, hash) << commrank << cmdargs.input.substr(hash + 1);
    cmdargs.input = ss.str();
    if (cmdargs.index != string("")) {
      hash = cmdargs.index.find('#');
      if (hash == string::npos) {
        if (commrank == 0) {
          cerr << "[ERROR] Index file must contain '#' to be replaced with processor rank." << endl;
        }
        return false;
      }
      ss.str(string(""));
      ss.clear();
      ss << cmdargs.index.substr(0, hash) << commrank << cmdargs.index.substr(hash + 1);
      cmdargs.index = ss.str();
    }
  }
  hash = cmdargs.output.find('#');
  if (hash == string::npos) {
    if (commsize > 1) {
      if (commrank == 0) {
        cerr << "[ERROR] Output file must contain '#' to be replaced with processor rank." << endl;
      }
      return false;
    }
  } else {
    stringstream ss (stringstream::in | stringstream::out);
    ss << cmdargs.output.substr(0, hash) << commrank << cmdargs.output.substr(hash + 1);
    cmdargs.output = ss.str();
  }

#if 0
  int z;
  for (z = 0; z < commrank; ++z) {
    MPI::COMM_WORLD.Barrier();
  }
  cerr << "===== Processor " << z << " Parameters =====" << endl;
  cerr << "Input file: " << cmdargs.input << endl;
  cerr << "Output file: " << cmdargs.output << endl;
  cerr << "Index file: " << cmdargs.index << endl;
  cerr << "Block size: " << cmdargs.block_size << endl;
  cerr << "Page size: " << cmdargs.page_size << endl;
  cerr << "Include header: " << cmdargs.include_header << endl;
  cerr << "Include footer: " << cmdargs.include_footer << endl;
  cerr << "Include checksum: " << cmdargs.include_checksum << endl;
  cerr << "Decompress: " << cmdargs.decompress << endl;
  cerr << "Allow splitting: " << cmdargs.allow_splitting << endl;
  cerr << "Print index: " << cmdargs.print_index << endl;
  cerr << "Single input: " << cmdargs.single_input << endl;
  for (; z < commsize; ++z) {
    MPI::COMM_WORLD.Barrier();
  }
#endif

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
  int rank = MPI::COMM_WORLD.Get_rank();
  if (rank == 0) {
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
  } else if (rank == 1) {
    cerr << "[INFO] Only one process is used to print an index." << endl;
  }
  return 0;
}

InputStream *makeInputStream(deque<uint64_t> *index) {
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  if (cmdargs.input == string("-")) {
    InputStream *is;
    NEW(is, IStream<istream>, cin);
    return is;
  } else if (!cmdargs.single_input) {
      InputStream *is;
      NEW(is, MPIPartialFileInputStream, MPI::COMM_SELF, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
      if (cmdargs.decompress) {
        NEW(is, LZOInputStream, is, index);
      }
      return is;
  } else if (commsize <= 1) {
    InputStream *is;
    NEW(is, MPIPartialFileInputStream, MPI::COMM_WORLD, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
    if (cmdargs.decompress) {
      NEW(is, LZOInputStream, is, index);
    }
    return is;
  } else if (cmdargs.decompress) {
    MPI::File index_file = MPI::File::Open(MPI::COMM_WORLD, cmdargs.index.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);
    MPI::Offset index_file_size = index_file.Get_size();
    MPI::Offset num_integers = index_file_size / sizeof(uint64_t);
    MPI::Offset num_chunks = num_integers - 1;
    MPI::Offset chunks_per_proc = num_chunks / commsize;
    MPI::Offset remaining_chunks = num_chunks % commsize;
    MPI::Offset mybegin = commrank * chunks_per_proc;
    MPI::Offset myend = mybegin + chunks_per_proc;
    if (commrank < remaining_chunks) {
      mybegin += commrank;
      myend += commrank + 1;
    } else {
      mybegin += remaining_chunks;
      myend += remaining_chunks;
    }
    mybegin *= sizeof(uint64_t);
    myend *= sizeof(uint64_t);
    uint64_t newbegin, newend;
    index_file.Read_at(mybegin, &newbegin, sizeof(uint64_t), MPI::BYTE);
    index_file.Read_at(myend, &newend, sizeof(uint64_t), MPI::BYTE);
    if (is_little_endian()) {
      reverse_bytes(newbegin);
      reverse_bytes(newend);
    }
    index_file.Close();
    mybegin = newbegin;
    myend = newend;
    InputStream *is;
    NEW(is, MPIPartialFileInputStream, MPI::COMM_WORLD, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, mybegin, myend);
    NEW(is, LZOInputStream, is, index, commrank != 0, true);
    return is;
  } else {
    MPI::File input_file = MPI::File::Open(MPI::COMM_WORLD, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);
    MPI::Offset input_file_size = input_file.Get_size();
    MPI::Offset bytes_per_proc = input_file_size / commsize;
    MPI::Offset remaining_bytes = input_file_size % commsize;
    MPI::Offset mybegin = commrank * bytes_per_proc;
    MPI::Offset myend = mybegin + bytes_per_proc;
    if (commrank < remaining_bytes) {
      mybegin += commrank;
      myend += commrank + 1;
    } else {
      mybegin += remaining_bytes;
      myend += remaining_bytes;
    }
    InputStream *is;
    input_file.Close();
    NEW(is, MPIPartialFileInputStream, MPI::COMM_WORLD, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, mybegin, myend);
    return is;
  }
}

OutputStream *makeOutputStream(deque<uint64_t> *index) {
  if (cmdargs.output == string("-")) {
    OutputStream *os;
    NEW(os, OStream<ostream>, cout);
    // compression not possible with output to stdout,
    // so not checking that case
    return os;
  }
  OutputStream *os;
  NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
  if (!cmdargs.decompress) {
    NEW(os, LZOOutputStream, os, index, cmdargs.block_size, cmdargs.include_header, cmdargs.include_footer, cmdargs.include_checksum);
    NEW(os, BufferedOutputStream, os, cmdargs.block_size, false);
  }
  return os;
}

OutputStream *makeIndexStream() {
  if (cmdargs.decompress || cmdargs.index == string("")) {
    return NULL;
  }
  OutputStream *os;
  NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.index.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
  return os;
}

int main(int argc, char **argv) {
  MPI::Init(argc, argv);
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  if (!parse_args(argc, argv)) {
    MPI::Finalize();
    return -1;
  }
  if (cmdargs.print_index) {
    int r = print_index();
    MPI::Finalize();
    return r;
  }
  DPtr<uint8_t> *nump = NULL;
  deque<uint64_t> *index = NULL;
  OutputStream *xs = makeIndexStream();
  if (xs != NULL) {
    NEW(index, deque<uint64_t>);
    NEW(nump, MPtr<uint8_t>, sizeof(uint64_t));
  }
  InputStream *is = makeInputStream(index);
  OutputStream *os = makeOutputStream(index);
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
  MPI::Finalize();
}
