#include "sys/sys.h"
#include "main/build-encoded-rules.cpp"

#ifdef USE_MPI
#undef USE_MPI
#warning "Ignoring USE_MPI in favor of automatic configuration."
#endif

#ifdef USE_SNAP
#undef USE_SNAP
#warning "Ignoring USE_SNAP in favor of automatic configuration."
#endif

#ifdef REL_CONFIG
#undef REL_CONFIG
#warning "Ignoring REL_CONFIG in favor of automatic configuration."
#endif

#ifdef REL_CONTAINER
#undef REL_CONTAINER
#warning "Ignoring REL_CONTAINER in favor of automatic configuration."
#endif

#ifdef TUPLE_SIZE
#undef TUPLE_SIZE
#warning "Ignoring TUPLE_SIZE in favor of automatic configuration."
#endif

#if SYSTEM == SYS_CCNI_OPTERONS || \
    SYSTEM == SYS_BLUE_GENE_L   || \
    SYSTEM == SYS_BLUE_GENE_P
#define REL_CONFIG 0
#define REL_CONTAINER deque
#define USE_MPI

#elif SYSTEM == SYS_MASTIFF       || \
      SYSTEM == SYS_BLUE_GENE_Q
#define REL_CONFIG 1
#define REL_CONTAINER deque
#define USE_MPI
#ifndef USE_POSIX_MEMALIGN
#define USE_POSIX_MEMALIGN
#endif

#elif SYSTEM == SYS_CRAY_XMT    || \
      SYSTEM == SYS_CRAY_XMT_2
#define REL_CONFIG 2
#define USE_SNAP
#define USE_MTGL

#else
#define REL_CONFIG 0
#define REL_CONTAINER deque

#endif

#if defined(USE_MPI)
#include <mpi.h>
#include "par/DistComputation.h"
#elif defined(USE_SNAP)
#include <snapshot/client.h>
#else
#include <sys/stat.h>
#endif

#include "rel/Relation.h"

#if REL_CONFIG != REL_MTA
#include "sys/endian.h"
using namespace sys;
#include "util/funcs.h"
using namespace util;
#endif

using namespace rel;
using namespace std;

typedef Tuple<constint_t> Tup;
typedef Relation<varint_t, constint_t> Rel;
typedef map<constint_t, Rel> PredMap;

// GLOBAL DATA
PredMap atoms;
Rel triples_pos;
#ifdef USE_INDEX_SPO
Rel triples_spo;
#endif
#ifdef USE_INDEX_OSP
Rel tripels_osp;
#endif

void *myalloc(size_t nbytes, size_t align) {
#ifdef USE_POSIX_MEMALIGN
  void *p = NULL;
  if (align < sizeof(void*))
    align = sizeof(void*);
  }
  size_t po2 = 0;
  size_t shifter = align;
  while (shifter > 1) {
    shifter >>= 1;
    ++po2;
  }
  if ((((size_t)1) << po2) != align) {
    ++po2;
  }
  size_t align = ((size_t)1) << po2;
  if (posix_memalign(&p, align, num_items * item_size) != 0) {
    return NULL;
  }
  return p;
#else
  return malloc(nbytes);
#endif
}

uint8_t *read_all(const char *filename, size_t &len) {
  #if defined(USE_MPI)
  {
    MPI::File file = MPI::File::Open(MPI::COMM_WORLD, filename,
                                     MPI::MODE_RDONLY | MPI::MODE_SEQUENTIAL,
                                     MPI::INFO_NULL);
    MPI::Offset filesize = file.Get_size();
    MPI::Offset bytesread = 0;
    uint8_t *buffer = (uint8_t*)myalloc(filesize, sizeof(constint_t));
    while (bytesread < filesize) {
      MPI::Status stat;
      file.Read_all(buffer + bytesread, filesize - bytesread, MPI::BYTE, stat);
      bytesread += stat.Get_count(MPI::BYTE);
    }
    file.Close();
    len = (size_t) filesize;
    return buffer;
  }
  #elif defined(USE_SNAP)
  {
    snap_stat_buf file_stat;
    int64_t snap_error;
    snap_stat(filename, SNAP_ANY_SW, &file_stat, &snap_error);
    len = file_stat.st_size;
    uint8_t *p = (uint8_t*)myalloc(len, sizeof(constint_t));
    snap_restore(filename, p, len, &snap_error);
    return p;
  }
  #else
  {
    struct stat filestatus;
    stat(filename, &filestatus);
    len = filestatus.st_size;
    uint8_t *p = (uint8_t*)myalloc(len, sizeof(constint_t));
    ifstream fin(filename);
    fin.read((char*)p, len);
    fin.close();
    return p;
  }
  #endif
}

template<typename Container>
void load_rules(const char *filename, Container &rules) {
  size_t len;
  uint8_t *begin = read_all(filename, len);
  const uint8_t *bytes = begin;
  const uint8_t *end = bytes + len;
  while (bytes != NULL && bytes != end) {
    Rule rule;
    bytes = build_rule(bytes, end, rule);
    rules.push_back(rule);
  }
  if (bytes == NULL) {
    cerr << "[ERROR] An error occurred while loading the rules." << endl;
    rules.clear();
  }
  free(begin);
}

bool load_data(const char *filename) {
  size_t len;
  uint8_t *bytes = NULL;
  #if defined(USE_MPI)
  {
    int rank = MPI::COMM_WORLD.Get_rank();
    int commsize = MPI::COMM_WORLD.Get_size();
    string fnamestr(filename);
    size_t hash = fnamestr.find('#');
    if (hash == string::npos && commsize > 1) {
      if (rank == 0) {
        cerr << "[ERROR] Data file name must contain a '#' to be replaced "
             << "with the processor rank.  Aborting." << endl;
      }
      return false;
    }
    if (hash != string::npos) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << fnamestr.substr(0, hash) << rank
         << fnamestr.substr(hash + 1, fnamestr.size() - hash - 1);
      fnamestr = ss.str();
    }
    MPI::File file = MPI::File::Open(MPI::COMM_SELF, filename,
        MPI::MODE_RDONLY | MPI::MODE_SEQUENTIAL | MPI::MODE_UNIQUE_OPEN,
        MPI::INFO_NULL);
    MPI::Offset filesize = file.Get_size();
    MPI::Offset bytesread = 0;
    // TODO come back and to asynchronous reading page-by-page while loading
    //      triples
    uint8_t *buffer = (uint8_t*)myalloc(filesize, sizeof(constint_t));
    while (bytesread < filesize) {
      MPI::Status stat;
      file.Read(buffer + bytesread, filesize - bytesread, MPI::BYTE, stat);
      bytesread += stat.Get_count(MPI::BYTE);
    }
    file.Close();
    len = (size_t) filesize;
  }
  #else
  {
    bytes = read_all(filename, len);
  }
  #endif
  if (len % (3*sizeof(constint_t)) != 0) {
    free(bytes);
    cerr << "[ERROR] Data file has unexpected length "
         << len << ", not an even multiple of "
         << (3*sizeof(constint_t)) << ".  Aborting." << endl;
    return false;
  }
  size_t ntriples = len / (3 * sizeof(constint_t));
  triples_pos.resize(ntriples);
  #if REL_CONFIG == REL_OMP
  {
    size_t i, j;
    #pragma omp parallel for default(shared) private(i, j)
    for (i = 0; i < ntriples; ++i) {
      const uint8_t *offset = bytes + (i * 3 * sizeof(constint_t));
      Tup triple(3);
      #pragma omp for default(shared) private(j)
      for (j = 0; j < 3; ++j) {
        memcpy(&triple[j], offset + j * sizeof(constint_t), sizeof(constint_t));
        if (is_little_endian()) {
          reverse_bytes(&triple[j]);
        }
      }
      triples_pos[i].absorb(triple);
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i, j;
    #pragma mta assert parallel
    for (i = 0; i < ntriples; ++i) {
      const uint8_t *offset = bytes + (i * 3 * sizeof(constint_t));
      Tup triple(3);
      #pragma mta assert parallel
      for (j = 0; j < 3; ++j) {
        memcpy(&triple[j], offset + j * sizeof(constint_t), sizeof(constint_t));
        // Cray XMT is big-endian, so don't worry about it.
      }
      triples_pos[i].absorb(triple);
    }
  }
  #else
  {
    size_t i, j;
    for (i = 0; i < ntriples; ++i) {
      const uint8_t *offset = bytes + (i * 3 * sizeof(constint_t));
      Tup triple(3);
      for (j = 0; j < 3; ++j) {
        memcpy(&triple[j], offset + j * sizeof(constint_t), sizeof(constint_t));
        if (is_little_endian()) {
          reverse_bytes(triple[j]);
        }
      }
      triples_pos[i].absorb(triple);
    }
  }
  #endif
  free(bytes);
  return true;
}

void index_data() {
  size_t nums[3];
  map<varint_t, size_t> attrs;
  attrs[(varint_t)0] = 0; attrs[(varint_t)1] = 1; attrs[(varint_t)2] = 2;
  triples_pos.name(attrs);
  #ifdef USE_INDEX_SPO
    triples_spo = triples_pos;
    nums[0] = 0; nums[1] = 1; nums[2] = 2;
    Order<constint_t> spo(nums, 3, false);
    Rel::uniq(triples_spo, spo);
  #endif
  #ifdef USE_INDEX_OSP
    map<varint_t, size_t>().swap(attrs);
    triples_osp = triples_pos;
    nums[0] = 2; nums[1] = 0; nums[2] = 1;
    Order<constint_t> osp(nums, 3, false);
    Rel::uniq(triples_osp, osp);
  #endif
  map<varint_t, size_t>().swap(attrs);
  nums[0] = 1; nums[1] = 2; nums[2] = 0;
  Order<constint_t> pos(nums, 3, false);
  Rel::uniq(triples_pos, pos);
}

inline
void unindex_data() {
  #ifdef USE_INDEX_SPO
  Rel().swap(triples_spo);
  #endif
  #ifdef USE_INDEX_OSP
  Rel().swap(triples_osp);
  #endif
}

inline
void drop_atoms() {
  PredMap().swap(atoms);
}

void write_data(const char *filename) {
  uint8_t *bytes = (uint8_t*)myalloc(triples_pos.size()*3*sizeof(constint_t), sizeof(constint_t));
  #if REL_CONFIG == REL_OMP
  {
    size_t i, j;
    #pragma omp parallel for default(shared) private(i, j)
    for (i = 0; i < triples_pos.size(); ++i) {
      if (is_little_endian()) {
        for (j = 0; j < 3; ++j) {
          reverse_bytes(triples_pos[i][j]);
        }
      }
      copy(triples_pos[i].begin(), triples_pos[i].end(),
           (constint_t*)(bytes + i*3*sizeof(constint_t)));
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i;
    #pragma mta assert parallel
    for (i = 0; i < triples_pos.size(); ++i) {
      // Cray XMT is big-endian, so don't worry about it
      copy(triples_pos[i].begin(), triples_pos[i].end(),
           (constint_t*)(bytes + i*3*sizeof(constint_t)));
    }
  }
  #else
  {
    size_t i, j;
    for (i = 0; i < triples_pos.size(); ++i) {
      if (is_little_endian()) {
        for (j = 0; j < 3; ++j) {
          reverse_bytes(triples_pos[i][j]);
        }
      }
      copy(triples_pos[i].begin(), triples_pos[i].end(),
           (constint_t*)(bytes + i*3*sizeof(constint_t)));
    }
  }
  #endif
  #if defined(USE_MPI)
  {
    int rank = MPI::COMM_WORLD.Get_rank();
    int nproc = MPI::COMM_WORLD.Get_size();
    string fnamestr(filename);
    size_t hash = fnamestr.find('#');
    if (hash == string::npos && commsize > 1) {
      if (rank == 0) {
        cerr << "[ERROR] File name must contain a '#' to be replaced with processor rank." << endl;
      }
      return;
    }
    if (hash != string::npos) {
      stringstream ss (stringstream::in | stringstream::out);
      ss << fnamestr.substr(0, hash) << rank << fnamestr.substr(hash + 1, fnamestr.size() - hash - 1);
      fnamestr = ss.str();
    }
    MPI::File file = MPI::File::Open(MPI::COMM_SELF, fnamestr.c_str(),
        MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL | MPI::MODE_UNIQUE_OPEN | MPI::MODE_SEQUENTIAL,
        MPI::INFO_NULL);
    file.Write(bytes, 3*sizeof(constint_t)*triples_pos.size(), MPI::BYTE);
    file.Close();
  }
  #elif defined(USE_SNAP)
  {
    int64_t snap_error;
    snap_snapshot(filename, bytes, 3*sizeof(constint_t)*triples_pos.size(), &snap_error);
  }
  #else
  {
    ofstream fout(filename);
    fout.write((char*)bytes, 3*sizeof(constint_t)*triples_pos.size());
    fout.close();
  }
  #endif
  free(bytes);
}

// If special returns true, then intermediate may be
// "destroyed" (of uncertain contents).  The result
// is in filtered.
bool special(Condition &condition, Rel &intermediate,
             Rel &filtered, bool sign) {
  if (condition.type != ATOMIC) {
    return false;
  }
  switch (condition.get.atom.type) {
    case ATOM:
    case MEMBERSHIP:
    case SUBCLASS:
    case FRAME:
      return false;
    case EQUALITY:
    case EXTERNAL:
      break; // handle after case statement
    default:
      cerr << "[ERROR] Unhandled case " << (int)condition.get.atom.type
           << " at line " << __LINE__ << endl;
      return true; // this should stop the query
  }
  Rel().swap(filtered);
  if (condition.get.atom.type == EQUALITY) {
    Term *lhs = &(condition.get.atom.get.sides[0]);
    Term *rhs = &(condition.get.atom.get.sides[1]);
    if (lhs->type == VARIABLE && rhs->type == CONSTANT) {
      std::swap(lhs, rhs);
    }
    if (rhs->type == CONSTANT) {
      if (( sign && lhs->get.constant == rhs->get.constant) ||
          (!sign && lhs->get.constant != rhs->get.constant)) {
        filtered.swap(intermediate);
      }
    } else if (lhs->type == CONSTANT) {
      bool successful = intermediate.select(
          &rhs->get.variable, &lhs->get.constant, 1,
          NULL, NULL, 0,
          sign, filtered);
      if (!successful) {
        cerr << "[ERROR] An error occurred in processing equality.\n";
      } else {
        filtered.swap(intermediate);
      }
    } else {
      bool successful = intermediate.select(
          NULL, NULL, 0,
          &lhs->get.variable, &rhs->get.variable, 1,
          sign, filtered);
      if (!successful) {
        cerr << "[ERROR] An error occurred in processing variable equality.\n";
      } else {
        filtered.swap(intermediate);
      }
    }
    return true;
  }

  // The rest is condition.get.atom.type == EXTERNAL
  Builtin &builtin = condition.get.atom.get.builtin;
  switch (builtin.predicate) {
    case BUILTIN_PRED_LIST_CONTAINS: {
      if (builtin.arguments.end - builtin.arguments.begin != 2) {
        cerr << "[ERROR] Invalid use of pred:list-contains which must have exactly two arguments, but instead found " << builtin << endl;
        return true;
      }
      Term *arg1 = builtin.arguments.begin;
      Term *arg2 = arg1 + 1;
      if (arg1->type != LIST) {
        cerr << "[ERROR] First argument of pred:list-contains shoulsd be a lit, but instead found " << *arg1 << endl;
        return true;
      }
      switch (arg2->type) {
        case FUNCTION:
        case LIST:
          cerr << "[ERROR] Only supporting constants or variables as second argument of pred:list-contains.\n";
          return true;
        case CONSTANT: {
          Term *t = arg1->get.termlist.begin;
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->get.constant == arg2->get.constant) {
              if (sign) {
                filtered.swap(intermediate);
              }
              break;
            }
          }
          if (!sign && t == arg1->get.termlist.end) {
            filtered.swap(intermediate);
          }
          return true;
        }
        case VARIABLE: {
          size_t nterms = arg1->get.termlist.end - arg1->get.termlist.begin;
          if (sign) {
            cerr << "[ERROR] The implementor got lazy and didn't support this.\n";
          } else {
            varint_t *vars = (varint_t*)myalloc(nterms * sizeof(varint_t), sizeof(varint_t));
            constint_t *vals = (constint_t*)myalloc(nterms * sizeof(constint_t), sizeof(constint_t));
            size_t i;
            for (i = 0; i < nterms; ++i) {
              vars[i] = arg2->get.variable;
              vals[i] = arg1->get.termlist.begin[i].get.constant;
            }
            intermediate.select(vars, vals, nterms, NULL, NULL, 0, false, filtered);
            free(vars);
            free(vals);
          }
          return true;
        }
        default:
          cerr << "[ERROR] Unhandled case " << (int)arg2->type << " at line " << __LINE__ << endl;
          return true;
      }
    }
    default: {
      cerr << "[ERROR] Builtin predicate " << (int)builtin.predicate << " is unsupported.\n";
      return true;
    }
  }
}

void select_atoms(Atom &atom, Rel &results) {
  Rel().swap(results);
  Rel &base = atoms[atom.predicate];
  if (base.empty()) {
    return;
  }
  map<varint_t, size_t> attrmap;
  map<varint_t, varint_t> renamings;
  size_t nargs = atom.arguments.end - atom.arguments.begin;
  varint_t *vars = (varint_t*)myalloc(nargs*sizeof(varint_t), sizeof(varint_t));
  constint_t *vals = (constint_t*)myalloc(nargs*sizeof(constint_t), sizeof(constint_t));
  varint_t *vars1 = (varint_t*)myalloc(nargs*sizeof(varint_t), sizeof(varint_t));
  varint_t *vars2 = (varint_t*)myalloc(nargs*sizeof(varint_t), sizeof(varint_t));
  size_t len1, len2;
  len1 = len2 = 0;
  size_t i;
  for (i = 0; i < nargs; ++i) {
    Term &t = atom.arguments.begin[i];
    if (t.type == CONSTANT) {
      vars[len1] = (varint_t)i;
      vals[len1] = t.get.constant;
      ++len1;
    } else if (t.type == VARIABLE) {
      map<varint_t, size_t>::const_iterator it = attrmap.find(t.get.variable);
      if (it == attrmap.end()) {
        renamings[(varint_t)i] = t.get.variable;
        attrmap[t.get.variable] = i;
      } else {
        vars1[len2] = it->second;
        vars2[len2] = (varint_t)i;
        ++len2;
      }
    } else {
      free(vars);
      free(vals);
      free(vars1);
      free(vars2);
      cerr << "[ERROR] Supporting only constants and variables as arguments in predicates.\n";
      return;
    }
  }
  bool successful = base.select(vars, vals, len1,
                                vars1, vars2, len2,
                                true, results);
  free(vars);
  free(vals);
  free(vars1);
  free(vars2);
  if (!successful) {
    cerr << "[ERROR] Selection of atoms was unsuccessful.\n";
    return;
  }
  successful = results.rename(renamings);
  if (!successful) {
    cerr << "[ERROR] Renaming failed at line " << __LINE__ << endl;
    return;
  }
  // Doing a projection here would save space but waste time.
#if 0
  cerr << "      " << results.size() << " atoms selected, with attributes:\n";
  map<varint_t, size_t>::const_iterator jt = results.attrs().begin();
  for (; jt != results.attrs().end(); ++jt) {
    cerr << "        " << (int)jt->first << " -> " << jt->second << endl;
  }
#endif
}

void select_frames(Frame &frame, Rel &results) {
  Rel().swap(results);
  results.name(triples_pos.attrs());
  if (frame.slots.end - frame.slots.begin != 1) {
    cerr << "[ERROR] Frames are supported only if they have exactly one slot.\n";
    return;
  }
  map<varint_t, size_t> attrmap;
  map<varint_t, varint_t> renamings;
  Term pattern[3];
  pattern[0] = frame.object;
  pattern[1] = frame.slots.begin->first;
  pattern[2] = frame.slots.begin->second;
  Tup lower(3);
  Tup upper(3);
  
  varint_t *vars = (varint_t*)myalloc(3*sizeof(varint_t), sizeof(varint_t));
  constint_t *vals = (constint_t*)myalloc(3*sizeof(constint_t), sizeof(constint_t));
  varint_t *vars1 = (varint_t*)myalloc(3*sizeof(varint_t), sizeof(varint_t));
  varint_t *vars2 = (varint_t*)myalloc(3*sizeof(varint_t), sizeof(varint_t));
  size_t len1, len2, i, j;
  len1 = len2 = 0;
  int idx = 0;
  for (i = 0; i < 3; ++i) {
    if (pattern[i].type == CONSTANT) {
      lower[i] = upper[i] = pattern[i].get.constant;
      idx |= (1 << (2-i));
      vars[len1] = (varint_t)i;
      vals[len1] = pattern[i].get.constant;
      ++len1;
    } else if (pattern[i].type == VARIABLE) {
      lower[i] = 0;
      upper[i] = numeric_limits<constint_t>::max();
      map<varint_t, size_t>::const_iterator it =
          attrmap.find(pattern[i].get.variable);
      if (it == attrmap.end()) {
        renamings[(varint_t)i] = pattern[i].get.variable;
        attrmap[pattern[i].get.variable] = i;
      } else {
        vars1[len2] = it->second;
        vars2[len2] = pattern[i].get.variable;
        ++len2;
      }
    } else {
      cerr << "[ERROR] Supporting only constant and variable terms in frames.\n";
      free(vars); free(vals); free(vars1); free(vars2);
      return;
    }
  }
  bool do_select = false;
  const Rel *rel_to_use = NULL;
  Order<constint_t> order;
  switch (idx) {
    case 0x0: // unbound
      cerr << "          Complete copy commencing..." << flush;
      results = triples_pos;
      results.rename(renamings);
      cerr << " done." << endl;
      return;
    case 0x2: // P
    case 0x3: // PO
    case 0x7: // POS
    {
      size_t ord[3] = { 1, 2, 0 };
      order = Order<constint_t>(ord, 3, false);
      rel_to_use = &triples_pos;
      break;
    }
    #ifdef USE_INDEX_SPO
    case 0x4: // S
    case 0x6: // SP
    {
      size_t ord[3] = { 0, 1, 2 };
      order = Order<constint_t>(ord, 3, false);
      rel_to_use = &triples_spo;
      break;
    }
    #endif
    #ifdef USE_INDEX_OSP
    case 0x1: // O
    case 0x5: // OS
    {
      size_t ord[3] = { 2, 0, 1 };
      order = Order<constint_t>(ord, 3, false);
      rel_to_use = &triples_osp;
      break;
    }
    #endif
    default: {
      do_select = true;
      rel_to_use = &triples_pos;
      break;
    }
  }
  if (do_select) {
    rel_to_use->select(vars, vals, len1, vars1, vars2, len2, true, results);
    results.rename(renamings);
  } else {
    size_t begin, end;
    begin = lower_bound(rel_to_use->begin(), rel_to_use->end(), lower, order)
            - rel_to_use->begin();
    end = upper_bound(rel_to_use->begin() + begin, rel_to_use->end(), upper, order)
          - rel_to_use->begin();
    Rel temp;
    temp.name(results.attrs());
    temp.resize(end - begin);
    #if REL_CONFIG == REL_OMP
    {
      #pragma omp parallel for default(shared) private(i)
      for (i = begin; i < end; ++i) {
        temp[i-begin] = (*rel_to_use)[i];
      }
    }
    #elif REL_CONFIG == REL_MTA
    {
      #pragma mta assert parallel
      for (i = begin; i < end; ++i) {
        temp[i-begin] = (*rel_to_use)[i];
      }
    }
    #else
    {
      copy(rel_to_use->begin() + begin,
           rel_to_use->begin() + end,
           temp.begin());
    }
    #endif
    results.swap(temp);
    results.rename(renamings);
  }
  free(vars); free(vals); free(vars1); free(vars2);
#if 0
  cerr << "      " << results.size() << " frames selected\n";
  map<varint_t, size_t>::const_iterator jt = results.attrs().begin();
  for (; jt != results.attrs().end(); ++jt) {
    cerr << "        " << (int)jt->first << " -> " << jt->second << endl;
  }
#endif
}

void query(Atomic &atom, Rel &results) {
  switch (atom.type) {
    case ATOM: {
      select_atoms(atom.get.atom, results);
      return;
    }
    case MEMBERSHIP:
    case SUBCLASS:
    case EQUALITY: {
      Rel().swap(results);
      return;
    }
    case EXTERNAL: {
      cerr << "[ERROR] This should never happen, at line " << __LINE__ << endl;
      return;
    }
    case FRAME: {
      select_frames(atom.get.frame, results);
      return;
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int) atom.type << " at line " << __LINE__ << endl;
      Rel().swap(results);
      return;
    }
  }
}

void query(Condition &condition, Rel &results) {
  Rel().swap(results);
  switch (condition.type) {
    case DISJUNCTION: {
      cerr << "[ERROR] Disjunction is unsupported.\n";
      return;
    }
    case EXISTENTIAL: {
      cerr << "[ERROR] Existential formulas are unsupported.\n";
      return;
    }
    case NEGATION: {
      cerr << "[ERROR] Negation is supported only on independent facts.\n";
      return;
    }
    case ATOMIC: {
      query(condition.get.atom, results);
      return;
    }
    case CONJUNCTION: {
      Rel intermediate;
      Condition *subformula = condition.get.subformulas.begin;
      for (; subformula != condition.get.subformulas.end; ++subformula) {
        cerr << "      Subformula " << (subformula - condition.get.subformulas.begin) << endl;
        Rel temp;
        if (subformula->type == NEGATION) {
          if (subformula == condition.get.subformulas.begin) {
            cerr << "[ERROR] First subformula in conjunction must not be negated.\n";
            return;
          }
          // TODO this will not work right unless all the variables
          // in the negated subformula are already bound.
          if (!special(subformula->get.subformulas.begin[0], intermediate, temp, false)) {
            cerr << "[ERROR] Negation is supported only on independent facts.\n";
            return;
          }
          intermediate.swap(temp);
          //cerr << "negatively filtered results " << intermediate.size() << endl;
          continue;
        }
        // TODO this will not work right if subformula is an equality
        // statement in which both sides are variables that are not
        // already bound.
        if (special(*subformula, intermediate, temp, true)) {
          intermediate.swap(temp);
          //cerr << "positively filtered results " << intermediate.size() << endl;
          continue;
        }
        cerr << "        Selecting..." << endl;
        query(*subformula, temp);
        cerr << "        Selected " << temp.size() << endl;
        if (subformula == condition.get.subformulas.begin) {
          intermediate.swap(temp);
          //cerr << "selected results " << intermediate.size() << endl;
        } else {
          cerr << "        Joining..." << endl;
          Rel subresults;
          //Rel::nested_loop_join(intermediate, temp, subresults);
          Rel::index_join(intermediate, temp, subresults);
          intermediate.swap(subresults);
          //cerr << "join results " << intermediate.size() << endl;
          #if 0
          map<varint_t, size_t>::const_iterator jt = intermediate.attrs().begin();
          for (; jt != intermediate.attrs().end(); ++jt) {
            cerr << "  " << (int)jt->first << " -> " << jt->second << endl;
          }
          #endif
        }
        if (intermediate.empty()) {
          break;
        }
      }
      results.swap(intermediate);
      return;
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int)condition.type << " on line " << __LINE__ << endl;
      return;
    }
  }
}

bool assert_atom(Atom &atom, const Rel &results) {
  cerr << "assert_atom called\n";
  if (results.empty()) {
    return false;
  }
  cerr << "assert_atom called with non-empty results\n";
  const map<varint_t, size_t> &attrs = results.attrs();
  size_t arity = atom.arguments.end - atom.arguments.begin;
  Rel &base = atoms[atom.predicate];
  if (base.empty()) {
    size_t i;
    map<varint_t, size_t> attrs;
    for (i = 0; i < arity; ++i) {
      attrs[(varint_t)i] = i;
    }
    base.name(attrs);
  }
  size_t offset = base.size();
  size_t nresults = results.size();
  base.resize(offset + nresults);
  #if REL_CONFIG == REL_OMP
  {
    size_t i, j;
    #pragma omp parallel for default(shared) private(i, j) nowait
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &tuple = base[offset + i];
      tuple.resize(arity);
      #pragma omp for default(shared) private(j) nowait
      for (j = 0; j < arity; ++j) {
        const Term &t = atom.arguments.begin[j];
        if (t.type == CONSTANT) {
          tuple[j] = t.get.constant;
        } else if (t.type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(t.get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          tuple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i, j;
    #pragma mta assert parallel
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &tuple = base[offset + i];
      tuple.resize(arity);
      #pragma mta assert parallel
      for (j = 0; j < arity; ++j) {
        const Term &t = atom.arguments.begin[j];
        if (t.type == CONSTANT) {
          tuple[j] = t.get.constant;
        } else if (t.type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(t.get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          tuple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #else
  {
    size_t i, j;
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &tuple = base[offset + i];
      tuple.resize(arity);
      for (j = 0; j < arity; ++j) {
        const Term &t = atom.arguments.begin[j];
        if (t.type == CONSTANT) {
          tuple[j] = t.get.constant;
        } else if (t.type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(t.get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          tuple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #endif
  return true;
}

bool assert_frame(Frame &frame, const Rel &results) {
  cerr << "assert_frame called\n";
  if (frame.slots.end - frame.slots.begin != 1) {
    cerr << "[ERROR] Supporting only frames with exactly one slot.\n";
    return false;
  }
  if (results.empty()) {
    return false;
  }
  cerr << "assert_frame called with non-empty results\n";
  Term pattern[3];
  pattern[0] = frame.object;
  pattern[1] = frame.slots.begin->first;
  pattern[2] = frame.slots.begin->second;
  const map<varint_t, size_t> &attrs = results.attrs();
  size_t offset = triples_pos.size();
  size_t nresults = results.size();
  triples_pos.resize(offset + nresults);
  #if REL_CONFIG == REL_OMP
  {
    size_t i, j;
    #pragma omp parallel for default(shared) private(i, j) nowait
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &triple = triples_pos[offset + i];
      triple.resize(3);
      #pragma omp for default(shared) private(j) nowait
      for (j = 0; j < 3; ++j) {
        if (pattern[j].type == CONSTANT) {
          triple[j] = pattern[j].get.constant;
        } else if (pattern[j].type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(pattern[j].get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          triple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #elif REL_CONFIG == REL_MTA
  {
    size_t i, j;
    #pragma mta assert parallel
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &triple = triples_pos[offset + i];
      triple.resize(3);
      #pragma mta assert parallel
      #pragma mta max concurrency 3
      for (j = 0; j < 3; ++j) {
        if (pattern[j].type == CONSTANT) {
          triple[j] = pattern[j].get.constant;
        } else if (pattern[j].type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(pattern[j].get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          triple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #else
  {
    size_t i, j;
    for (i = 0; i < nresults; ++i) {
      const Tup &result = results[i];
      Tup &triple = triples_pos[offset + i];
      triple.resize(3);
      for (j = 0; j < 3; ++j) {
        if (pattern[j].type == CONSTANT) {
          triple[j] = pattern[j].get.constant;
        } else if (pattern[j].type == VARIABLE) {
          map<varint_t, size_t>::const_iterator it = attrs.find(pattern[j].get.variable);
          if (it == attrs.end()) {
            cerr << "[ERROR] Expected variable not found in results.\n";
          }
          triple[j] = result[it->second];
        } else {
          cerr << "[ERROR] Unsupported term type in results.  Should never happen.\n";
        }
      }
    }
  }
  #endif
  #if defined(USE_INDEX_SPO) || defined(USE_INDEX_OSP)
    #ifdef USE_INDEX_SPO
      triples_spo.resize(offset + nresults);
    #endif
    #ifdef USE_INDEX_OSP
      triples_osp.resize(offset + nresults);
    #endif
    #if REL_CONFIG == REL_OMP
    {
      size_t i;
      #pragma omp parallel for default(shared) private(i) nowait
      for (i = 0; i < nresults; ++i) {
        #ifdef USE_INDEX_SPO
          triples_spo[offset+i] = triples_pos[offset+i];
        #endif
        #ifdef USE_INDEX_OSP
          triples_osp[offset+i] = triples_pos[offset+i];
        #endif
      }
    }
    #endif
  #endif
  return true;
}

bool act(ActionBlock &action_block, const Rel &results) {
  if (action_block.action_variables.begin != action_block.action_variables.end) {
    cerr << "[ERROR] Action variables are unsupported.\n";
    return false;
  }
  if (results.empty()) {
    return false;
  }
  bool maybe_changed = false;
  Action *action = action_block.actions.begin;
  for (; action != action_block.actions.end; ++action) {
    if (action->type != ASSERT_FACT) {
      cerr << "[ERROR] Supporting only assertion actions.\n";
      return maybe_changed;
    }
    if (action->get.atom.type == ATOM) {
      maybe_changed = assert_atom(action->get.atom.get.atom, results) || maybe_changed;
      continue;
    }
    if (action->get.atom.type == FRAME) {
      maybe_changed = assert_frame(action->get.atom.get.frame, results) || maybe_changed;
      continue;
    }
    cerr << "[ERROR] Only atoms and frames can be asserted.  This should never happen.\n";
    return maybe_changed;
  }
  return maybe_changed;
}

void note_sizes(size_t &numtriples, map<constint_t, size_t> &sizes) {
  numtriples = triples_pos.size();
  map<constint_t, size_t> sz;
  map<constint_t, Rel>::const_iterator it = atoms.begin();
  for (; it != atoms.end(); ++it) {
    sz[it->first] = it->second.size();
  }
  sizes.swap(sz);
}

bool maybe_pred_changed(const map<constint_t, size_t> &sizes, set<constint_t> &preds) {
  set<constint_t> temp;
  map<constint_t, size_t>::const_iterator it = sizes.begin();
  for (; it != sizes.end(); ++it) {
    if (atoms[it->first].size() != it->second) {
      temp.insert(it->first);
    }
  }
  preds.swap(temp);
  return !preds.empty();
}

bool sizes_changed(const size_t numtriples, const map<constint_t, size_t> &sizes) {
  if (numtriples != triples_pos.size()) {
    return true;
  }
  map<constint_t, Rel>::const_iterator it = atoms.begin();
  for (; it != atoms.end(); ++it) {
    map<constint_t, size_t>::const_iterator it2 = sizes.find(it->first);
    if (it2 == sizes.end()) {
      return true;
    }
    if (it->second.size() != it2->second) {
      return true;
    }
  }
  return false;
}

#ifdef USE_MPI
#define INFER_REPORT(x)
#define MAIN_REPORT(x) if (MPI::COMM_WORLD.Get_rank() == 0) cerr << x
#else
#define INFER_REPORT(x) cerr << x
#define MAIN_REPORT(x) cerr << x
#endif

bool infer(Rule &rule) {
  Rel results;
  size_t numtriples;
  map<constint_t, size_t> sizes;
  note_sizes(numtriples, sizes);
  query(rule.condition, results);
  INFER_REPORT("      " << results.size() << " results\n");
  bool maybe_changed = act(rule.action_block, results);
  if (!maybe_changed) {
    cerr << "According to act, nothing changed.\n";
    return false;
  }
  cerr << "According to act, things might have changed.\n";
  if (triples_pos.size() != numtriples) {
    cerr << "Triples might have changed.\n";
    index_data();
  }
  set<constint_t> preds;
  if (maybe_pred_changed(sizes, preds)) {
    Order<constint_t> lexical_order;
    set<constint_t>::const_iterator it = preds.begin();
    for (; it != preds.end(); ++it) {
      cerr << "Pred " << (int)*it << " might have changed.\n";
      Rel::uniq(atoms[*it], lexical_order);
    }
  }
  bool retval = sizes_changed(numtriples, sizes);
  if (retval) {
    cerr << "And really, things did change.\n";
  } else {
    cerr << "But really, nothing changed.\n";
  }
  return retval;
}

template<typename Container>
void infer(Container &rules) {
  size_t rules_since_change = 0;
  size_t cycle_count;
  for (cycle_count = 0; rules_since_change < rules.size(); ++cycle_count) {
    INFER_REPORT("Cycle " << (cycle_count + 1) << endl);
    size_t rule_count;
    for (rule_count = 0; rule_count < rules.size() && rules_since_change < rules.size(); ++rule_count) {
      INFER_REPORT("  Rule " << (rule_count + 1) << endl);
      bool changed = true;
      size_t app_count;
      for (app_count = 0; changed; ++app_count) {
        INFER_REPORT("    Application " << (app_count + 1) << endl);
        changed = infer(rules[rule_count]);
      }
      if (app_count > 1) {
        rules_since_change = 1;
      } else {
        ++rules_since_change;
      }
    }
  }
}

int inconsistent() {
  #ifdef USE_MPI
    int incon = atoms[CONST_RIF_ERROR].empty() ? 0 : 1;
    int num_incon;
    MPI::COMM_WORLD.Allreduce(&incon, &num_incon, 1, MPI::INT, MPI::SUM);
    return num_incon;
  #else
    return atoms[CONST_RIF_ERROR].empty() ? 0 : 1;
  #endif
}


// HERE BEGINS A LOT OF MPI-SPECIFIC STUFF //
#ifdef USE_MPI
uint32_t hash_jenkins_one_at_a_time(Tup &tuple, size_t *order, size_t len) {
  uint32_t h = 0;
  size_t i, j;
  for (i = 0; i < len; ++i) {
    for (j = 0; j < sizeof(constint_t); ++j) {
      uint8_t b = ((tuple[order[i]] >> (j << 3)) & 0xFF);
      h += b;
      h += (h << 10);
      h ^= (h >> 6);
    }
  }
  h += (h << 3);
  h ^= (h >> 11);
  h += (h << 15);
  return h;
}

class Redistributor : public DistComputation {
protected:
  const int rank, nproc;
  Rel::const_iterator it, end;
public:
  Rel new_triples_pos;
  Redistributor(Distributor *dist, const int rank, const int nproc)
      throw(BaseException<void*>)
      : DistComputation(dist), rank(rank), nproc(nproc) {
    // do nothing
  }
  virtual ~Redistributor() throw(DistException) {
    // do nothing
  }
  void start() throw(TraceableException) {
    size_t ord[3] = { 1, 2, 0 };
    Order order(ord, 3, false);
    Rel(order).swap(this->new_triples_pos);
    this->it = triples_pos.begin();
    this->end = triples_pos.end();
  }
  void finish() throw(TraceableException) {
    // do nothing
  }
  void fail() throw() {
    cerr << "[ERROR] Processor " << this->rank << " experienced a failure when attempting to hash distribute triples.\n";
  }
  int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) {
    if (this->it == this->end) {
      return -2;
    }
    uint32_t send_to = this->route(*this->it);
    ++this->it;
    if (this->rank == send_to) {
      this->new_triples_pos.add(triple);
      return -1;
    }
    len = 3 * sizeof(constint_t);
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    constint_t *write_to = (constint_t*)buffer->dptr();
    copy(triple.begin(), triple.end(), write_to);
    return (int)send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    Triple triple(3);
    const constint_t *read_from = (constint_t*) msg->ptr();
    copy(read_from, read_from + 3*sizeof(constint_t), triple.begin());
    this->new_triples_pos.add(triple);
  }
  virtual int route(const Tup &tuple) = 0;
};

class HashDistributor : public Redistributor {
private:
  const size_t *order;
  size_t len;
public:
  HashDistributor(Distributor *dist, const int rank, const int nproc,
                  const size_t *order, size_t len)
      throw(BaseException<void*>)
      : Redistributor(dist, rank, nproc), order(order), len(len) {
    // do nothing
  }
  virtual ~HashDistributor() throw(DistException) {
    // do nothing
    // note that this means HashDistributor does NOT own this->order
  }
  int route(const Tup &tuple) {
    hash_jenkins_one_at_a_time(tuple, this->order, this->len) % this->nproc;
  }
}

class RandomDistributor : public Redistributor {
public:
  RandomDistributor(Distributor *dist, const int rank, const int nproc)
      throw(BaseException<void*>)
      : Redistributor(dist, rank, nproc) {
    // do nothing
  }
  virtual ~RandomDistributor() throw(DistException) {
    // do nothing
  }
  int route(const Tup &tuple) {
    return random() % this->nproc;
  }
};

void randomize_data() {
  Distributor *dist;
  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, 3*sizeof(constint_t),
      cmdargs.num_requests, cmdargs.check_every, (int)__LINE__);
  RandomDistributor *randdist;
  NEW(randdist, RandomDistributor, dist, MPI::COMM_WORLD.Get_rank(),
                                         MPI::COMM_WORLD.Get_size());
  randdist->exec();
  triples_pos.swap(randdist->new_triples_pos);
  DELETE(randdist);
}

void uniq_data() {
  size_t ord[3] = { 0, 1, 2 };
  Distributor *dist;
  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, 3*sizeof(constint_t),
      cmdargs.num_requests, cmdargs.check_every, (int)__LINE__);
  HashDistributor *hashdist;
  NEW(hashdist, HashDistributor, dist, MPI::COMM_WORLD.Get_rank(),
                MPI::COMM_WORLD.Get_size(), ord, 3);
  hashdist->exec();
  triples_pos.swap(hashdist->new_triples_pos);
  DELETE(hashdist);
}

void replicate_data(const char *filename) {
  if (filename == NULL || MPI::COMM_WORLD.Get_size() <= 1) {
    return;
  }
  MAIN_REPORT("Replicating data meeting conditions in " << filename << endl);
  size_t len;
  uint8_t *replication_conditions = read_all(filename, len);
  if (replication_conditions == NULL || len <= 0) {
    MAIN_REPORT("No conditions in " << filename << endl);
    return;
  }
  Rel switcheroo;
  const uint8_t *bytes = replication_conditions;
  const uint8_t *end = bytes + len;
  while (bytes != end) {
    Condition condition;
    bytes = build_condition(bytes, end, condition);
    if (condition.type != CONJUNCTION) {
      MAIN_REPORT("[ERROR] Invalid replication pattern.  Should be a conjunction but isn't.\n");
      return;
    }
    if (condition.get.subformulas.begin->type != ATOMIC) {
      MAIN_REPORT("[ERROR] Invalid replication pattern.  First subformula should be atomic but isn't.\n");
      return;
    }
    if (condition.get.subformulas.begin->get.atom.type != FRAME &&
        condition.get.subformulas.begin->get.atom.type != ATOM) {
      continue;
    }
    Rel results;
    query(condition, results);
    Action action;
    action.type = ASSERT_FACT;
    action.get.atom = condition.get.subformulas.begin->get.atom;
    ActionBlock action_block;
    action_block.begin = action_block.end = NULL;
    action_block.actions.begin = &action;
    action_block.actions.end = &action + 1;
    triples_pos.swap(switcheroo);
    act(action_block, results);
    triples_pos.swap(switcheroo);
  }
  free(replication_conditions);

  int rank = MPI::COMM_WORLD.Get_rank();
  int nproc = MPI::COMM_WORLD.Get_size();

  int size = switcheroo.size() * 3 * sizeof(constint_t);
  bytes = (uint8_t*)myalloc(size, sizeof(constint_t));
  int *sizes = (int*)myalloc(nproc*sizeof(int), sizeof(int));
  int *displs = (int*)myalloc(nproc*sizeof(int), sizeof(int));
  size_t i, j;
  #pragma omp parallel for default(shared) private(i, j)
  for (i = 0; i < switcheroo.size(); ++i) {
    for (j = 0; j < 3; ++j) {
      memcpy(bytes + (3*i+j)*sizeof(constint_t), &switcheroo[i][j], sizeof(constint_t));
    }
  }
  MPI::COMM_WORLD.Allgather(&size, 1, MPI::INT, sizes, 1, MPI::INT);
  displs[0] = 0;
  for (i = 1; i < nproc; ++i) {
    displs[i] = displs[i-1] + sizes[i-1];
  }
  size_t recvbytes = displs[nproc-1] + sizes[nproc-1];
  uint8_t *recvbuf = (uint8_t*)myalloc(recvbytes, sizeof(constint_t));
  MPI::COMM_WORLD.Allgatherv(bytes, size, MPI::BYTE, recvbuf, sizes, displs, MPI::BYTE);
  free(bytes);

  MAIN_REPORT("Loading replicated data.");
  size_t offset = triples_pos.size();
  triples_pos.resize(offset + (recvbytes/(3*sizeof(constint_t)) - sizes[rank]));
  #pragma omp parallel default(shared) private(i)
  {
    #pragma omp for nowait
    for (i = 0; i < displs[rank]; i += 3*sizeof(constint_t)) {
      triples_pos[offset+i].resize(3);
      copy((constint_t*)(recvbuf + 3*i*sizeof(constint_t)),
           (constint_t*)(recvbuf + 3*(i+1)*sizeof(constint_t)),
           triples_pos[offset+i].begin());
    }
    #pragma omp for nowait
    for (i = displs[rank] + sizes[rank]; i < recvbytes; i += 3*sizeof(constint_t)) {
      triples_pos[offset+i].resize(3);
      copy((constint_t*)(recvbuf + 3*i*sizeof(constint_t)),
           (constint_t*)(recvbuf + 3*(i+1)*sizeof(constint_t)),
           triples_pos[offset+i-sizes[rank]].begin());
    }
  }
  free(sizes);
  free(displs);
  free(recvbuf);
}
#endif

struct cmdargs_t {
  char *rules;
  char *input;
  char *output;
  char *repls;
  size_t num_requests;
  size_t check_every;
  bool randomize;
  bool uniq;
  bool complete;
} cmdargs = {
  NULL,   // rules
  NULL,   // input
  NULL,   // output
  NULL,   // repls
  8,      // num_requests
  4096,   // check_every
  false,  // randomize
  false,  // uniq
  false,  // complete
};

bool parse_args(int argc, char **argv) {
  MAIN_REPORT(argv[0]);
  size_t i;
  for (i = 1; i < argc; ++i) {
    MAIN_REPORT(' ' << argv[i]);
  }
  MAIN_REPORT(endl);
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--num-requests") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.num_requests;
    } else if (strcmp(argv[i], "--check-every") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> cmdargs.check_every;
    } else if (strcmp(argv[i], "--uniq") == 0 || strcmp(argv[i], "--unique") == 0) {
      cmdargs.uniq = true;
    } else if (strcmp(argv[i], "--randomize") == 0) {
      cmdargs.randomize = true;
    } else if (strcmp(argv[i], "--complete") == 0) {
      cmdargs.complete = true;
    } else if (cmdargs.rules == NULL) {
      cmdargs.rules = argv[i];
    } else if (cmdargs.input == NULL) {
      cmdargs.input = argv[i];
    } else if (cmdargs.output == NULL) {
      cmdargs.output = argv[i];
    } else if (cmdargs.repls == NULL) {
      cmdargs.repls = argv[i];
    } else {
      MAIN_REPORT("[WARNING] Ignoring unrecognized command-line argument: " << argv[i] << endl);
    }
  }
  return cmdargs.rules != NULL && cmdargs.input != NULL && cmdargs.output != NULL;
}

int init(int argc, char **argv) {
  #if defined(USE_MPI)
  {
    int required;
    #if REL_CONFIG == REL_OMP
      required = MPI_THREAD_FUNNELED;
    #else
      required = MPI_THREAD_SINGLE;
    #endif
    int provided = MPI::Init_thread(argc, argv, required);
    if (provided < required) {
      MAIN_REPORT("The required threading support is not provided by the present implementation of MPI.  Aborting." << endl);
      return (int) __LINE__;
    }
    srandom(MPI::COMM_WORLD.Get_rank());
  }
  #elif defined(USE_SNAP)
  snap_init();
  #endif
  if (!parse_args(argc, argv)) {
    return (int) __LINE__;
  }
  return 0;
}

#ifdef USE_MPI
#define abort_if_not_zero(rc)                     \
  if (rc != 0) {                                  \
    cerr << "Aborting with code " << rc << endl;  \
    MPI::COMM_WORLD.Abort(rc);                    \
  }
#else
#define abort_if_not_zero(rc)                     \
  if (rc != 0) {                                  \
    cerr << "Aborting with code " << rc << endl;  \
    return rc;                                    \
  }
#endif

void final() {
  #ifdef USE_MPI
    MPI::Finalize();
  #endif
}

int main(int argc, char **argv) {

  int r = init(argc, argv);
  abort_if_not_zero(r)

  MAIN_REPORT("Loading rules..." << endl);
  vector<Rule> rules;
  load_rules(cmdargs.rules, rules);

  MAIN_REPORT("Loading data..." << endl);
  load_data(cmdargs.input);

  #ifdef USE_MPI
  if (cmdargs.randomize) {
    MAIN_REPORT("Randomly distributed data..." << endl);
    randomize_data();
  }
  if (cmdargs.repls != NULL) {
    MAIN_REPORT("Replicating data..." << endl);
    replicate_data(cmdargs.repls);
  }
  #endif

  MAIN_REPORT("Building indexes..." << endl);
  index_data();

  MAIN_REPORT("Inferring..." << endl);
  infer(rules);

  MAIN_REPORT("Destroying indexes..." << endl);
  unindex_data();

  #ifdef USE_MPI
  if (cmdargs.complete) {
    size_t numtriples = triples_pos.size();
    MAIN_REPORT("Replicating data again..." << endl);
    replicate_data(cmdargs.repls);
    uint8_t iter = numtriples == triples_pos.size() ? 0 : 1;
    uint8_t another_iteration;
    MPI::COMM_WORLD.Allreduce(&iter, &another_iteraton, 1, MPI::BYTE, MPI::BOR);
    uint8_t niters;;
    for (niters = 1; another_iteration != 0; ++niters) {
      numtriples = triple_pos.size();
      MAIN_REPORT("Building indexes again..." << endl);
      index_data();
      MAIN_REPORT("Inferring again..." << endl);
      infer(rules);
      MAIN_REPORT("Replicating data again..." << endl);
      replicate_data(cmdargs.repls);
      MAIN_REPORT("Destroying indexes again..." << endl);
      unindex_data();
      iter = numtriples == triples_pos.size() ? 0 : 1;
      MPI::COMM_WORLD.Allreduce(&iter, &another_iteraton, 1, MPI::BYTE, MPI::BOR);
    }
    MAIN_REPORT(niters << " embarrassingly parallel iterations were required to reach completion." << endl);
  }
  #endif

  r = inconsistent();
  if (r > 0) {
    #ifdef USE_MPI
    MAIN_REPORT(r << " processors reported inconsistency." << endl);
    #endif
    MAIN_REPORT("INCONSISTENT" << endl);
  }

  MAIN_REPORT("Dropping atoms..." << endl);
  drop_atoms();

  MAIN_REPORT("Writing data..." << endl);
  write_data(cmdargs.output);

  final();
  return 0;
}
