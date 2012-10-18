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

#include <algorithm>
#include <iostream>
#include <map>
#include <mpi.h>
#include <string>
#include "iri/IRIRef.h"
#include "io/IFStream.h"
#include "io/OFStream.h"
#include "lang/LangTag.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIDistPtrFileOutputStream.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"
#include "sys/char.h"

#ifndef COLLECT_STATS
#define COLLECT_STATS 1
#endif

using namespace iri;
using namespace io;
using namespace lang;
using namespace par;
using namespace rdf;
using namespace std;

#define ONCE_BARRIER_BEGIN \
  MPI::COMM_WORLD.Barrier(); \
  if (rank == 0) {

#define ONCE_BARRIER_END \
  } \
  MPI::COMM_WORLD.Barrier();

#define STAT_NUM_INPUT_TRIPLES                       0
#define STAT_NUM_ERROR_TRIPLES                       1
#define STAT_NUM_OUTPUT_TRIPLES                      2
#define STAT_NUM_IRIS                                3
#define STAT_NUM_IRIS_NORMALIZED                     4
#define STAT_NUM_IRIS_URIFIED                        5
#define STAT_NUM_IRIS_NORMALIZED_AND_URIFIED         6
#define STAT_NUM_IRIS_WITH_HTTP_SCHEME               7
#define STAT_NUM_IRIS_WITH_HTTPS_SCHEME              8
#define STAT_NUM_IRIS_WITH_USER_INFO                 9
#define STAT_NUM_IRIS_WITH_HOST                     10
#define STAT_NUM_IRIS_WITH_PORT                     11
#define STAT_NUM_IRIS_WITH_QUERY                    12
#define STAT_NUM_IRIS_WITH_FRAGMENT                 13
#define STAT_NUM_IRIS_SLASH                         14
#define STAT_NUM_IRIS_HASH                          15
#define STAT_NUM_BNODES                             16
#define STAT_NUM_BNODES_ALNUM_LABELLED              17
#define STAT_NUM_BNODES_NFC_LABELLED                18
#define STAT_NUM_LITERALS                           19
#define STAT_NUM_SIMPLE_LITERALS                    20
#define STAT_NUM_LANG_LITERALS                      21
#define STAT_NUM_TYPED_LITERALS                     22
#define STAT_NUM_LITERALS_WITH_NFC_LEXICALS         23
#define STAT_NUM_LANG_NORMALIZED                    24
#define STAT_NUM_LANG_EXTLANGIFIED                  25
#define STAT_NUM_LANG_NORMALIZED_AND_EXTLANGIFIED   26
#define STAT_NUM_LANG_WITH_SCRIPT                   27
#define STAT_NUM_LANG_WITH_REGION                   28
#define STAT_NUM_LANG_WITH_VARIANTS                 29
#define STAT_NUM_LANG_WITH_EXTENSIONS               30
#define STAT_NUM_LANG_WITH_PRIVATE_USE              31
#define STAT_NUM_LANG_REGULAR_GRANDFATHERED         32
#define STAT_NUM_LANG_IRREGULAR_GRANDFATHERED       33
#define STAT_NUM_MALFORMED_IRI_ERROR                34
#define STAT_NUM_MALFORMED_LANG_ERROR               35
#define STAT_NUM_INVALID_UTF8_ENCODING_ERROR        36
#define STAT_NUM_INVALID_UCS_CODEPOINT_ERROR        37
#define STAT_NUM_OTHER_ERROR                        38

#if COLLECT_STATS
#define STAT_NUM_CODES                              39
#else
#define STAT_NUM_CODES                               3
#endif

#define STATOUT cout
#define PRINT(stat) STATOUT << #stat << ": " << totalstats[stat] << endl;

unsigned long stats[STAT_NUM_CODES];

#if COLLECT_STATS
RDFTerm collect_stats(RDFTerm term) {
  switch (term.getType()) {
  case BNODE: {
    ++stats[STAT_NUM_BNODES];
    DPtr<uint8_t> *label = term.getLabel();
    const uint8_t *mark = label->dptr();
    const uint8_t *end = mark + label->size();
    bool alnum = true;
    for (; mark != end && alnum; ++mark) {
      alnum = is_alnum(*mark);
    }
    if (alnum) {
      ++stats[STAT_NUM_BNODES_ALNUM_LABELLED];
    }
    RDFTerm norm = term.normalize();
    if (norm.equals(term)) {
      ++stats[STAT_NUM_BNODES_NFC_LABELLED];
    }
    label->drop();
    return norm;
  }
  case IRI: {
    ++stats[STAT_NUM_IRIS];
    IRIRef iriref = term.getIRIRef();
    DPtr<uint8_t> *part = iriref.getPart(SCHEME);
    const uint8_t *mark = part->dptr();
    bool slash = true;
    bool hash = true;
    if (part->size() == 4 &&
        to_lower(mark[0]) == to_ascii('h') &&
        to_lower(mark[1]) == to_ascii('t') &&
        to_lower(mark[2]) == to_ascii('t') &&
        to_lower(mark[3]) == to_ascii('p')) {
      ++stats[STAT_NUM_IRIS_WITH_HTTP_SCHEME];
    } else if (part->size() == 5 &&
               to_lower(mark[0]) == to_ascii('h') &&
               to_lower(mark[1]) == to_ascii('t') &&
               to_lower(mark[2]) == to_ascii('t') &&
               to_lower(mark[3]) == to_ascii('p') &&
               to_lower(mark[4]) == to_ascii('s')) {
      ++stats[STAT_NUM_IRIS_WITH_HTTPS_SCHEME];
    } else {
      slash = false;
      hash = false;
    }
    part->drop();
    part = iriref.getPart(USER_INFO);
    if (part != NULL) {
      ++stats[STAT_NUM_IRIS_WITH_USER_INFO];
      part->drop();
    }
    part = iriref.getPart(HOST);
    if (part != NULL) {
      ++stats[STAT_NUM_IRIS_WITH_HOST];
      part->drop();
    }
    part = iriref.getPart(PORT);
    if (part != NULL) {
      ++stats[STAT_NUM_IRIS_WITH_PORT];
      part->drop();
    }
    part = iriref.getPart(QUERY);
    if (part != NULL) {
      ++stats[STAT_NUM_IRIS_WITH_QUERY];
      part->drop();
    }
    part = iriref.getPart(FRAGMENT);
    if (part != NULL) {
      ++stats[STAT_NUM_IRIS_WITH_FRAGMENT];
      part->drop();
      slash = false;
    } else {
      hash = false;
    }
    if (slash) {
      part = iriref.getPart(PATH);
      mark = part->dptr();
      const uint8_t *end = mark + part->size();
      for (; mark != end && *mark != to_ascii('/'); ++mark) {
        // check for slash in path
      }
      slash &= (mark != end);
      part->drop();
    }
    if (hash) {
      ++stats[STAT_NUM_IRIS_HASH];
    }
    if (slash) {
      ++stats[STAT_NUM_IRIS_SLASH];
    }
    IRIRef normiri(iriref);
    normiri.normalize();
    bool normed = iriref.equals(normiri);
    if (normed) {
      ++stats[STAT_NUM_IRIS_NORMALIZED];
    }
    IRIRef urif(normiri);
    urif.urify();
    if (iriref.equals(urif)) {
      ++stats[STAT_NUM_IRIS_URIFIED];
      if (normed) {
        ++stats[STAT_NUM_IRIS_NORMALIZED_AND_URIFIED];
      }
    }
    return RDFTerm(normiri);
  }
  case SIMPLE_LITERAL: {
    ++stats[STAT_NUM_LITERALS];
    ++stats[STAT_NUM_SIMPLE_LITERALS];
    RDFTerm norm(term);
    norm.normalize();
    if (norm.equals(term)) {
      ++stats[STAT_NUM_LITERALS_WITH_NFC_LEXICALS];
    }
    return norm;
  }
  case LANG_LITERAL: {
    ++stats[STAT_NUM_LITERALS];
    ++stats[STAT_NUM_LANG_LITERALS];
    LangTag lang = term.getLangTag();
    
    DPtr<uint8_t> *part = lang.getPart(SCRIPT);
    if (part != NULL) {
      ++stats[STAT_NUM_LANG_WITH_SCRIPT];
      part->drop();
    }
    part = lang.getPart(REGION);
    if (part != NULL) {
      ++stats[STAT_NUM_LANG_WITH_REGION];
      part->drop();
    }
    part = lang.getPart(VARIANTS);
    if (part != NULL) {
      ++stats[STAT_NUM_LANG_WITH_VARIANTS];
      part->drop();
    }
    part = lang.getPart(EXTENSIONS);
    if (part != NULL) {
      ++stats[STAT_NUM_LANG_WITH_EXTENSIONS];
      part->drop();
    }
    part = lang.getPart(PRIVATE_USE);
    if (part != NULL) {
      ++stats[STAT_NUM_LANG_WITH_PRIVATE_USE];
      part->drop();
    }
    LangTag normlang(lang);
    normlang.normalize();
    bool normed = lang.equals(normlang);
    if (normed) {
      ++stats[STAT_NUM_LANG_NORMALIZED];
    }
    LangTag extlang(normlang);
    extlang.extlangify();
    if (lang.equals(extlang)) {
      ++stats[STAT_NUM_LANG_EXTLANGIFIED];
      if (normed) {
        ++stats[STAT_NUM_LANG_NORMALIZED_AND_EXTLANGIFIED];
      }
    }
    if (lang.isRegularGrandfathered()) {
      ++stats[STAT_NUM_LANG_REGULAR_GRANDFATHERED];
    }
    if (lang.isIrregularGrandfathered()) {
      ++stats[STAT_NUM_LANG_IRREGULAR_GRANDFATHERED];
    }
    part = term.getLexForm();
    RDFTerm term2(part, &normlang);
    part->drop();
    RDFTerm norm(term2);
    norm.normalize();
    if (norm.equals(term2)) {
      ++stats[STAT_NUM_LITERALS_WITH_NFC_LEXICALS];
    }
    return norm;
  }
  case TYPED_LITERAL: {
    ++stats[STAT_NUM_LITERALS];
    ++stats[STAT_NUM_TYPED_LITERALS];
    IRIRef dtiri = term.getDatatype();
    IRIRef dtnorm = collect_stats(RDFTerm(dtiri)).getIRIRef();
    DPtr<uint8_t> *lex = term.getLexForm();
    RDFTerm term2(lex, dtnorm);
    lex->drop();
    RDFTerm norm(term2);
    norm.normalize();
    if (norm.equals(term2)) {
      ++stats[STAT_NUM_LITERALS_WITH_NFC_LEXICALS];
    }
    return norm;
  }
  }
}
#endif

#if COLLECT_STATS
RDFTriple collect_stats(RDFTriple triple) {
  RDFTerm subj = collect_stats(triple.getSubj());
  RDFTerm pred = collect_stats(triple.getPred());
  RDFTerm obj = collect_stats(triple.getObj());
  return RDFTriple(subj, pred, obj);
}
#endif

int main (int argc, char **argv) {

  MPI::Init(argc, argv);
  int rank = MPI::COMM_WORLD.Get_rank();

  fill(stats, stats + STAT_NUM_CODES, 0);

  const char *outfilename;

  ONCE_BARRIER_BEGIN
    STATOUT << "Executing with " << MPI::COMM_WORLD.Get_size() << " processors.\n";
  ONCE_BARRIER_END

  map<string, size_t> errors;
  int i;
  NTriplesWriter *ntw = NULL;
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == '\0') {
      if (i < argc - 1) {
        ++i;
        OutputStream *ofs;
        outfilename = argv[i];

        ONCE_BARRIER_BEGIN
          STATOUT << "Writing to " << outfilename << endl;
        ONCE_BARRIER_END

        if (ntw != NULL) {
          ntw->close();
          DELETE(ntw);
        }
        NEW(ofs, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, outfilename, MPI::MODE_WRONLY | MPI::MODE_CREATE, MPI::INFO_NULL, 1024*1024, true);
        NEW(ntw, NTriplesWriter, ofs);
      }
      continue;
    }

    ONCE_BARRIER_BEGIN
      STATOUT << "Normalizing " << argv[i] << endl;
    ONCE_BARRIER_END

    InputStream *ifs;

    ONCE_BARRIER_BEGIN
      STATOUT << "Reading from " << argv[i] << endl;
    ONCE_BARRIER_END

    NEW(ifs, MPIDelimFileInputStream, MPI::COMM_WORLD, argv[i], MPI::MODE_RDONLY, MPI::INFO_NULL, 1024 * 1024, to_ascii('\n'));
    NTriplesReader ntr(ifs);
    RDFTriple triple;
    for (;;) {
      try {
        if (!ntr.read(triple)) {

          // Have to do this to avoid deadlock.
          // Requires specifying new output file for each input file.
          // Real inconvenience.
          if (ntw != NULL) {
            ntw->close();
            DELETE(ntw);
            ntw = NULL;
          }

          ntr.close();
          break;
        }
#if COLLECT_STATS
        RDFTriple norm = collect_stats(triple);
#else
        RDFTriple norm(triple);
        norm.normalize();
#endif
        if (ntw == NULL) {
          cout << norm;
        } else {
          ntw->write(norm);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_OUTPUT_TRIPLES];
#if COLLECT_STATS
      } catch (MalformedIRIRefException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_ERROR_TRIPLES];
        ++stats[STAT_NUM_MALFORMED_IRI_ERROR];
        continue;
      } catch (MalformedLangTagException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_ERROR_TRIPLES];
        ++stats[STAT_NUM_MALFORMED_LANG_ERROR];
        continue;
      } catch (InvalidEncodingException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_ERROR_TRIPLES];
        ++stats[STAT_NUM_INVALID_UTF8_ENCODING_ERROR];
        continue;
      } catch (InvalidCodepointException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_ERROR_TRIPLES];
        ++stats[STAT_NUM_INVALID_UCS_CODEPOINT_ERROR];
        continue;
#endif
      } catch (TraceableException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        ++stats[STAT_NUM_INPUT_TRIPLES];
        ++stats[STAT_NUM_ERROR_TRIPLES];
#if COLLECT_STATS
        ++stats[STAT_NUM_OTHER_ERROR];
#endif
        continue;
      }
    }
  }
  if (ntw != NULL) {
    ntw->close();
    DELETE(ntw);
  }
  if (rank > 0) {
    int nothing;
    MPI::COMM_WORLD.Recv(&nothing, 1, MPI::INT, rank - 1, 18);
  }
  cerr << "===== ERROR SUMMARY =====" << endl;
  map<string, size_t>::iterator it = errors.begin();
  for (; it != errors.end(); ++it) {
    cerr << "\t[" << it->second << "] " << it->first;
  }
  cerr << endl;
  if (rank < MPI::COMM_WORLD.Get_size() - 1) {
    int nothing;
    MPI::COMM_WORLD.Send(&nothing, 1, MPI::INT, rank + 1, 18);
  }
  unsigned long totalstats[STAT_NUM_CODES];
  MPI::COMM_WORLD.Reduce(stats, totalstats, STAT_NUM_CODES, MPI::UNSIGNED_LONG, MPI::SUM, 0);
  if (rank == 0) {
    STATOUT << "===== SUMMARY =====" << endl;
    PRINT(STAT_NUM_INPUT_TRIPLES);
    PRINT(STAT_NUM_ERROR_TRIPLES);
    PRINT(STAT_NUM_OUTPUT_TRIPLES);
#if COLLECT_STATS
    PRINT(STAT_NUM_IRIS);
    PRINT(STAT_NUM_IRIS_NORMALIZED);
    PRINT(STAT_NUM_IRIS_URIFIED);
    PRINT(STAT_NUM_IRIS_NORMALIZED_AND_URIFIED);
    PRINT(STAT_NUM_IRIS_WITH_HTTP_SCHEME);
    PRINT(STAT_NUM_IRIS_WITH_HTTPS_SCHEME);
    PRINT(STAT_NUM_IRIS_WITH_USER_INFO);
    PRINT(STAT_NUM_IRIS_WITH_HOST);
    PRINT(STAT_NUM_IRIS_WITH_PORT);
    PRINT(STAT_NUM_IRIS_WITH_QUERY);
    PRINT(STAT_NUM_IRIS_WITH_FRAGMENT);
    PRINT(STAT_NUM_IRIS_SLASH);
    PRINT(STAT_NUM_IRIS_HASH);
    PRINT(STAT_NUM_BNODES);
    PRINT(STAT_NUM_BNODES_ALNUM_LABELLED);
    PRINT(STAT_NUM_BNODES_NFC_LABELLED);
    PRINT(STAT_NUM_LITERALS);
    PRINT(STAT_NUM_SIMPLE_LITERALS);
    PRINT(STAT_NUM_LANG_LITERALS);
    PRINT(STAT_NUM_TYPED_LITERALS);
    PRINT(STAT_NUM_LITERALS_WITH_NFC_LEXICALS);
    PRINT(STAT_NUM_LANG_NORMALIZED);
    PRINT(STAT_NUM_LANG_EXTLANGIFIED);
    PRINT(STAT_NUM_LANG_NORMALIZED_AND_EXTLANGIFIED);
    PRINT(STAT_NUM_LANG_WITH_SCRIPT);
    PRINT(STAT_NUM_LANG_WITH_REGION);
    PRINT(STAT_NUM_LANG_WITH_VARIANTS);
    PRINT(STAT_NUM_LANG_WITH_EXTENSIONS);
    PRINT(STAT_NUM_LANG_WITH_PRIVATE_USE);
    PRINT(STAT_NUM_LANG_REGULAR_GRANDFATHERED);
    PRINT(STAT_NUM_LANG_IRREGULAR_GRANDFATHERED);
    PRINT(STAT_NUM_MALFORMED_IRI_ERROR);
    PRINT(STAT_NUM_MALFORMED_LANG_ERROR);
    PRINT(STAT_NUM_INVALID_UTF8_ENCODING_ERROR);
    PRINT(STAT_NUM_INVALID_UCS_CODEPOINT_ERROR);
    PRINT(STAT_NUM_OTHER_ERROR);
#endif
    ASSERTNPTR(0);
  }

  // Added this barrier to prevent finalize from hanging with MVAPICH2.
  MPI::COMM_WORLD.Barrier();
  MPI::Finalize();
}
