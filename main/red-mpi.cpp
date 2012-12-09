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
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIDistPtrFileOutputStream.h"
#include "par/MPIPartialFileInputStream.h"
#include "rdf/RDFDictEncReader.h"
#include "rdf/RDFDictEncWriter.h"
#include "rdf/RDFDictionary.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"
#include "sys/endian.h"
#include "sys/ints.h"
#include "util/funcs.h"

using namespace io;
using namespace par;
using namespace ptr;
using namespace rdf;
using namespace std;
using namespace sys;
using namespace util;

#ifndef NBYTES
#define NBYTES 8
#endif

typedef RDFID<NBYTES> ID;
typedef RDFEncoder<ID> ENC;

struct cmdargs_t {
  string input;
  string input_format;
  string input_dict;
  string input_index;
  string output;
  string output_format;
  string output_dict;
  string output_index;
  size_t page_size;
  size_t block_size;
  bool single_input;
  bool single_output;
  bool global_dict;
} cmdargs = {
  /* input          */  string(""),
  /* input_format   */  string(""),
  /* input_dict     */  string(""),
  /* input_index    */  string(""),
  /* output         */  string(""),
  /* output_format  */  string(""),
  /* output_dict    */  string(""),
  /* output_index   */  string(""),
  /* page_size      */  0,
  /* block_size     */  0,
  /* single_input   */  false,
  /* single_output  */  false,
  /* global_dict    */  false,
};

size_t parse_size_t(char *cstr) {
  size_t sz;
  stringstream ss(stringstream::in | stringstream::out);
  ss << cstr;
  ss >> sz;
  return sz;
}

bool insert_processor_rank(const bool doit, string &str) {
  int commsize = MPI::COMM_WORLD.Get_size();
  if (!doit || str == string("")) {
    return true;
  }
  int rank = MPI::COMM_WORLD.Get_rank();
  size_t hash = str.find('#');
  if (hash == string::npos) {
    if (rank == 0 && commsize > 1) cerr << "[ERROR] File name specified should have a '#' in it to be replaced with processor rank." << endl;
    return commsize <= 1;
  }
  stringstream ss(stringstream::in | stringstream::out);
  ss << str.substr(0, hash) << rank << str.substr(hash + 1);
  str = ss.str();
  return true;
}

#define CMDARG(arg, flag1, flag2, cmd, def, val) \
  if (strcmp(arg, flag1) == 0 || strcmp(arg, flag2) == 0) { \
    if (cmdargs.cmd != def) { \
      if (rank == 0) cerr << "[ERROR] Can specify only one " << flag1 << " (or " << flag2 << ")." << endl; \
      return false; \
    } \
    cmdargs.cmd = val; \
  }

#define DEFAULTVAL(cmd, init, def) \
  if (cmdargs.cmd == init) { \
    cmdargs.cmd = def; \
  }

#define REQUIREDVAL(cmd, flag1, flag2, init) \
  if (cmdargs.cmd == init) { \
    if (rank == 0) cerr << "[ERROR] Must specify " << flag1 << " (or " << flag2 << ")." << endl; \
    return false; \
  }

#define CONDITIONALVAL(doit, cmd1, val1, cmd2, flag1, flag2, init) \
  if (doit && cmdargs.cmd1 == val1) { \
    REQUIREDVAL(cmd2, flag1, flag2, init) \
  }

#define ENUMVAL(cmd, flag1, flag2, options) \
  if (cmdargs.cmd.find(' ') != string::npos || options.find(cmdargs.cmd) == string::npos) { \
    if (rank == 0) cerr << "[ERROR] The values for " << flag1 << " (or " << flag2 << ") must be one of: " << options << endl; \
  }

bool parse_args(int argc, char **argv) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  int i;
  for (i = 1; i < argc; ++i) {
    CMDARG(argv[i], "--input", "-i", input, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--input-format", "-if", input_format, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--input-dict", "-id", input_dict, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--input-index", "-ix", input_index, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--output", "-o", output, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--output-format", "-of", output_format, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--output-dict", "-od", output_dict, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--output-index", "-ox", output_index, string(""), string(argv[++i]))
    else CMDARG(argv[i], "--page-size", "-p", page_size, 0, parse_size_t(argv[++i]))
    else CMDARG(argv[i], "--block-size", "-b", block_size, 0, parse_size_t(argv[++i]))
    else CMDARG(argv[i], "--single-input", "-si", single_input, false, true)
    else CMDARG(argv[i], "--single-output", "-so", single_output, false, true)
    else CMDARG(argv[i], "--global-dict", "-gd", global_dict, false, true)
    else { cerr << "[ERROR] Unrecognized flag: " << argv[i] << endl; return false; }
  }
  REQUIREDVAL(input, "--input", "-i", string(""))
  DEFAULTVAL(input_format, string(""), string("nt"))
  ENUMVAL(input_format, "--input-format", "-if", string("nt nt.lzo der"))
  CONDITIONALVAL(!cmdargs.single_input, input_format, string("der"), input_dict, "--input-dict", "-id", string(""))
  else CONDITIONALVAL(!cmdargs.single_input && commsize > 1, input_format, string("nt.lzo"), input_dict, "--input-index", "-ix", string(""))
  REQUIREDVAL(output, "--output", "-o", string(""))
  DEFAULTVAL(output_format, string(""), string("nt"))
  ENUMVAL(output_format, "--output-format", "-if", string("nt nt.lzo der"))
  CONDITIONALVAL(true, output_format, string("der"), output_dict, "--output-dict", "-od", string(""))
  DEFAULTVAL(page_size, 0, 4096)
  DEFAULTVAL(block_size, 0, 4096)
  return (insert_processor_rank(!cmdargs.single_input, cmdargs.input) &
          insert_processor_rank(!cmdargs.single_input, cmdargs.input_dict) &
          insert_processor_rank(!cmdargs.single_input, cmdargs.input_index) &
          insert_processor_rank(!cmdargs.single_output, cmdargs.output) &
          insert_processor_rank(!cmdargs.single_output, cmdargs.output_dict) &
          insert_processor_rank(!cmdargs.single_output, cmdargs.output_index))
         | commsize <= 1;
}

RDFReader *makeRDFReader() {
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  if (cmdargs.input_format == string("nt")) {
    if (cmdargs.single_input && commsize > 1) {
      InputStream *is;
      NEW(is, MPIDelimFileInputStream, MPI::COMM_WORLD, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, (uint8_t)'\n');
      RDFReader *rr;
      NEW(rr, NTriplesReader, is);
      return rr;
    } else {
      InputStream *is;
      NEW(is, MPIPartialFileInputStream, MPI::COMM_SELF, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
      RDFReader *rr;
      NEW(rr, NTriplesReader, is);
      return rr;
    }
  } else if (cmdargs.input_format == string("nt.lzo")) {
    if (cmdargs.single_input && commsize > 1) {
      // TODO
      if (commrank == 0) cerr << "[WARNING] Input nt.lzo must not have triples split across compressed blocks." << endl;
      MPI::File index_file = MPI::File::Open(MPI::COMM_WORLD, cmdargs.input_index.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);
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
      NEW(is, LZOInputStream, is, NULL, commrank != 0, true);
      RDFReader *rr;
      NEW(rr, NTriplesReader, is);
      return rr;
    } else {
      InputStream *is;
      NEW(is, MPIPartialFileInputStream, MPI::COMM_SELF, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
      NEW(is, LZOInputStream, is, NULL);
      RDFReader *rr;
      NEW(rr, NTriplesReader, is);
      return rr;
    }
  } else if (cmdargs.input_format == string("der")) {
    if (cmdargs.single_input && commsize > 1) {
      // TODO Come back and do this with a collective input stream for the dictionary
      // TODO and then divide up the input file just like the index file in LZO
      if (commrank == 0) cerr << "[ERROR] Single input der file is currently unsupported." << endl;
      return NULL;
    } else {
      InputStream *is;
      NEW(is, MPIPartialFileInputStream, MPI::COMM_SELF, cmdargs.input_dict.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
      RDFDictionary<ID, ENC> *dict = RDFDictEncReader<ID, ENC>::readDictionary(is, NULL);
      is->close();
      DELETE(is);
      NEW(is, MPIPartialFileInputStream, MPI::COMM_SELF, cmdargs.input.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL, cmdargs.page_size, 0, -1);
      RDFReader *rr;
      NEW(rr, WHOLE(RDFDictEncReader<ID, ENC>), is, dict, true, true);
      return rr;
    }
  }
  return NULL;
}

RDFWriter *makeRDFWriter(RDFDictionary<ID, ENC> *dict, deque<uint64_t> *index) {
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  if (cmdargs.output_format == string("nt")) {
    if (cmdargs.single_output && commsize > 1) {
      OutputStream *os;
      NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, true);
      RDFWriter *rw;
      NEW(rw, NTriplesWriter, os);
      return rw;
    } else {
      OutputStream *os;
      NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
      RDFWriter *rw;
      NEW(rw, NTriplesWriter, os);
      return rw;
    }
  } else if (cmdargs.output_format == string("nt.lzo")) {
    if (cmdargs.single_output && commsize > 1) {
      OutputStream *os;
      NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, true);
      NEW(os, LZOOutputStream, os, index, cmdargs.block_size, commrank == 0, commrank == commsize - 1, commsize == 1, false);
      NEW(os, BufferedOutputStream, os, cmdargs.block_size, false);
      RDFWriter *rw;
      NEW(rw, NTriplesWriter, os);
      return rw;
    } else {
      OutputStream *os;
      NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
      NEW(os, LZOOutputStream, os, index, cmdargs.block_size, true, true, true, false);
      RDFWriter *rw;
      NEW(rw, NTriplesWriter, os);
      return rw;
    }
  } else if (cmdargs.output_format == string("der")) {
    if (cmdargs.single_output && commsize > 1) {
      // Special case
      return NULL;
    } else if (cmdargs.global_dict && commsize > 1) {
      // Special case
      return NULL;
    } else {
      OutputStream *os;
      NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
      RDFWriter *rw;
      NEW(rw, WHOLE(RDFDictEncWriter<ID, ENC>), os, dict, false);
      return rw;
    }
  }
  return NULL;
}

int dictionary_encode() {
  int commrank = MPI::COMM_WORLD.Get_rank();
  if (commrank == 0) cerr << "[ERROR] Dictionary encoding with any global context (including --global-dict) is currently unsupported." << endl;
  return -1;
}

int main(int argc, char **argv) {
  MPI::Init(argc, argv);
  int commrank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  if (!parse_args(argc, argv)) {
    MPI::Finalize();
    return -1;
  }
#if 1
  cerr << "Input: " << cmdargs.input << endl;
  cerr << "Input format: " << cmdargs.input_format << endl;
  cerr << "Input dict: " << cmdargs.input_dict << endl;
  cerr << "Input index: " << cmdargs.input_index << endl;
  cerr << "Output: " << cmdargs.output << endl;
  cerr << "Output format: " << cmdargs.output_format << endl;
  cerr << "Output dict: " << cmdargs.output_dict << endl;
  cerr << "Output index: " << cmdargs.output_index << endl;
  cerr << "Page size: " << cmdargs.page_size << endl;
  cerr << "Block size: " << cmdargs.block_size << endl;
  cerr << "Single input: " << cmdargs.single_input << endl;
  cerr << "Single output: " << cmdargs.single_output << endl;
  cerr << "Global dict: " << cmdargs.global_dict << endl;
#endif
  if (cmdargs.output_format == string("der") && commsize > 1 &&
      (cmdargs.global_dict || cmdargs.single_output)) {
    int ret = dictionary_encode();
    MPI::Finalize();
    return ret;
  }
  deque<uint64_t> *index = NULL;
  OutputStream *xs = NULL;
  if (cmdargs.output_format == string("nt.lzo") && cmdargs.output_index != string("")) {
    NEW(index, deque<uint64_t>);
    if (commsize <= 1 || !cmdargs.single_output) {
      NEW(xs, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output_index.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
    }
  }
  RDFDictionary<ID, ENC> *dict = NULL;
  if (cmdargs.output_format == string("der") && cmdargs.output_dict != string("")) {
    NEW(dict, WHOLE(RDFDictionary<ID, ENC>));
  }
  RDFReader *rr = makeRDFReader();
  RDFWriter *rw = makeRDFWriter(dict, index);
  if (xs == NULL) {
    RDFTriple triple;
    while (rr->read(triple)) {
      rw->write(triple);
    }
    triple = RDFTriple();
    rr->close();
    rw->close();
    DELETE(rr);
    DELETE(rw);
    if (index != NULL) {
      unsigned long partial = index->back();
      if (commrank < commsize - 1) {
        index->pop_back();
      }
      unsigned long offset = 0;
      MPI::COMM_WORLD.Scan(&partial, &offset, 1, MPI::UNSIGNED_LONG, MPI::SUM);
      offset -= partial;
      deque<uint64_t>::iterator it = index->begin();
      for (; it != index->end(); ++it) {
        *it += offset;
      }
      partial = index->size();
      MPI::COMM_WORLD.Scan(&partial, &offset, 1, MPI::UNSIGNED_LONG, MPI::SUM);
      offset -= partial;
      // TODO come back to this and use MPIOffsetFileOutputStream
      if (commrank == 0) cerr << "[WARNING] LZO index output to a single file is currently unsupported.  Data will be compressed, but no index file will be output.";
      // TODO this is where to write out the index to the file starting at offset*sizeof(uint64_t)
      DELETE(index);
    }
  } else {
    DPtr<uint8_t> *nump;
    NEW(nump, MPtr<uint8_t>, sizeof(uint64_t));
    RDFTriple triple;
    while (rr->read(triple)) {
      rw->write(triple);
      if (!index->empty()) {
        deque<uint64_t>::iterator it = index->begin();
        for (; it != index->end(); ++it) {
          if (is_little_endian()) {
            reverse_bytes(*it);
          }
          memcpy(nump->dptr(), &*it, sizeof(uint64_t));
          xs->write(nump);
        }
        deque<uint64_t> swapper;
        index->swap(swapper);
      }
    }
    rr->close();
    rw->close();
    deque<uint64_t>::iterator it = index->begin();
    for (; it != index->end(); ++it) {
      if (is_little_endian()) {
        reverse_bytes(*it);
      }
      memcpy(nump->dptr(), &*it, sizeof(uint64_t));
      xs->write(nump);
    }
    xs->close();
    DELETE(rr);
    DELETE(rw);
    DELETE(xs);
    DELETE(index);
  }
  if (dict != NULL) {
    NEW(xs, MPIDistPtrFileOutputStream, MPI::COMM_SELF, cmdargs.output_dict.c_str(), MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL, cmdargs.page_size, false);
    RDFDictEncWriter<ID, ENC>::writeDictionary(xs, dict);
    xs->close();
    DELETE(xs);
    DELETE(dict);
  }
  ASSERTNPTR(0);
  MPI::Finalize();
  return 0;
}
