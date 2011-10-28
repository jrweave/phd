#include "ucs/nf.h"

#include <algorithm>
#include <new>
#include <sstream>
#include <vector>
#include "ptr/APtr.h"

#ifdef UCS_PLAY_DUMB
#warning "No UCS normalization will actually occur since UCS_PLAY_DUMB is defined.  This is probably for performance optimization.  Be sure that this is the desired behavior.\n"
#endif

#ifdef UCS_TRUST_CODEPOINTS
#warning "UCS codepoints will not be validated since UCS_TRUST_CODEPOINTS is defined.  This is probably for performance optimization.  Be sure that this is the desired behavior.\n"
#endif

namespace ucs {

using namespace ex;
using namespace ptr;
using namespace std;

InvalidCodepointException::InvalidCodepointException(const char *file,
    const unsigned int line, const uint32_t codepoint) throw()
    : TraceableException(file, line), codepoint(codepoint) {
  // do nothing
}

InvalidCodepointException::~InvalidCodepointException() throw() {
  // do nothing
}

const char *InvalidCodepointException::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << "\tcaused by invalid codepoint 0x"
      << hex << this->codepoint << "\n";
  return ss.str().c_str();
}

#include "ucs_arrays.cpp"

#ifndef UCS_PLAY_DUMB
const uint32_t *nflookupd(const uint32_t codepoint)
    throw(InvalidCodepointException) {
  const uint32_t *ub = upper_bound(UCS_DECOMPOSITION_RANGES,
      UCS_DECOMPOSITION_RANGES + UCS_DECOMPOSITION_RANGES_LEN, codepoint);
  if (ub == UCS_DECOMPOSITION_RANGES) {
    if (!nfvalid(codepoint)) {
      THROW(InvalidCodepointException, codepoint);
    }
    return NULL;
  }
  ub--;
  uint32_t offset = ub - UCS_DECOMPOSITION_RANGES;
  if ((offset & UINT32_C(1)) != UINT32_C(0)) {
    if (!nfvalid(codepoint)) {
      THROW(InvalidCodepointException, codepoint);
    }
    return NULL;
  }
  offset = UCS_DECOMPOSITION_OFFSETS[offset >> 1] + (codepoint - *ub);
  return UCS_DECOMPOSITIONS[offset];
}
#endif /* UCS_PLAY_DUMB */

#ifndef UCS_PLAY_DUMB
bool nfcmpccc(const uint32_t a, const uint32_t b) throw() {
  return UCS_UNPACK_CCC(a) < UCS_UNPACK_CCC(b);
}
#endif /* UCS_PLAY_DUMB */

#if defined(UCS_TRUST_CODEPOINTS) || defined(UCS_PLAY_DUMB)
bool nfvalid(const uint32_t codepoint) throw() {
  return true;
}

bool nfvalid(DPtr<uint32_t> *codepoints) throw(SizeUnknownException) {
  if (!codepoints->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  size_t i;
  for (i = 0; i < codepoints->size(); i++) {
    if (!nfvalid((*codepoints)[i])) {
      return false;
    }
  }
  return true;
}
#else
bool nfvalid(const uint32_t codepoint) throw() {
  const uint32_t *ub = upper_bound(UCS_CODEPOINT_RANGES,
      UCS_CODEPOINT_RANGES + UCS_CODEPOINT_RANGES_LEN, codepoint);
  if (ub == UCS_CODEPOINT_RANGES) {
    return false;
  }
  ub--;
  uint32_t offset = ub - UCS_CODEPOINT_RANGES;
  return (offset & UINT32_C(1)) == UINT32_C(0);
}
#endif /* UCS_TRUST_CODEPOINTS */

#ifndef UCS_PLAY_DUMB
DPtr<uint32_t> *nfreturn(vector<uint32_t> *vec) THROWS(BadAllocException) {
  APtr<uint32_t> *p = new APtr<uint32_t>(vec->size());
  size_t i;
  for (i = 0; i < p->size(); i++) {
    (*p)[i] = UCS_UNPACK_CODEPOINT(vec->at(i));
  }
  delete vec;
  return p;
}
TRACE(BadAllocException, "(trace)")
#endif /* UCS_PLAY_DUMB */

#ifndef UCS_PLAY_DUMB
vector<uint32_t> *nfdecompose(const DPtr<uint32_t> *codepoints,
    const bool use_compat)
    THROWS (InvalidCodepointException, SizeUnknownException,
    BadAllocException, bad_alloc) {
  if (!codepoints->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  size_t i;
  size_t max = codepoints->size();
  const DPtr<uint32_t> &cps = *codepoints;
  vector<uint32_t> *decomp;
  try {
    decomp = new vector<uint32_t>();
    decomp->reserve(codepoints->size());
  } RETHROW_BAD_ALLOC
  for (i = 0; i < max; i++) {
    const uint32_t *d = nflookupd(cps[i]);
    if (d == NULL) {
      decomp->push_back(cps[i]);
      continue;
    }
    if (use_compat) {
      const uint32_t len = UCS_DECOMP_COMPAT_LEN(d);
      const uint32_t *c = UCS_DECOMP_COMPAT_CHARS(d);
      decomp->insert(decomp->end(), c, c + len);
    } else {
      const uint32_t len = UCS_DECOMP_CANON_LEN(d);
      const uint32_t *c = UCS_DECOMP_CANON_CHARS(d);
      decomp->insert(decomp->end(), c, c + len);
    }
  }
  vector<uint32_t>::iterator it;
  for (it = decomp->begin(); it != decomp->end(); it++) {
    if (UCS_UNPACK_CCC(*it) > 0) {
      vector<uint32_t>::iterator start = it;
      for (; it != decomp->end() && UCS_UNPACK_CCC(*it) > 0; it++) {
        // do nothing
      }
      // since combining class is in upper 8 bits,
      // this will sort by combining class
      stable_sort(start, it, nfcmpccc);
      it--;
    }
  }
  return decomp;
}
TRACE(BadAllocException, "Couldn't allocate memory for UCS decomposition.")
#endif

#ifndef UCS_PLAY_DUMB
uint8_t nfqc(const DPtr<uint32_t> *codepoints, size_t *pos, bool do_c, bool do_k)
    throw (InvalidCodepointException, SizeUnknownException) {
  if (!codepoints->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  uint8_t lastccc = 0;
  uint8_t result = UCS_QC_YES;
  size_t i;
  for (i = 0; i < codepoints->size(); i++) {
    const uint32_t codepoint = (*codepoints)[i];
    const uint32_t *d = nflookupd(codepoint);
    uint8_t ccc;
    if (d == NULL) {
      if (!nfvalid(codepoint)) {
        THROW(InvalidCodepointException, codepoint);
      }
      ccc = 0;
    } else {
      ccc = (uint8_t) UCS_DECOMP_CCC(d);
    }
    if (lastccc > ccc && ccc != UINT8_C(0)) {
      if (pos != NULL) {
        *pos = i;
      }
      return UCS_QC_NO;
    }
    uint8_t check;
    if (d == NULL) {
      check = UCS_QC_YES;
    } else {
      if (do_c) {
        if (do_k) {
          check = (uint8_t) UCS_DECOMP_NFKC_QC(d);
        } else {
          check = (uint8_t) UCS_DECOMP_NFC_QC(d);
        }
      } else {
        if (do_k) {
          check = (uint8_t) UCS_DECOMP_NFKD_QC(d);
        } else {
          check = (uint8_t) UCS_DECOMP_NFD_QC(d);
        }
      }
    }
    if (check == UCS_QC_NO) {
      if (pos != NULL) {
        *pos = i;
      }
      return UCS_QC_NO;
    }
    if (check == UCS_QC_MAYBE && result != UCS_QC_MAYBE) {
      if (pos != NULL) {
        *pos = i;
      }
      result = UCS_QC_MAYBE;
    }
    lastccc = ccc;
  }
  return result;
}
#endif /* UCS_PLAY_DUMB */

uint8_t nfd_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw(InvalidCodepointException, SizeUnknownException) {
  #ifdef UCS_PLAY_DUMB
  return UCS_QC_YES;
  #else
  try {
    return nfqc(codepoints, pos, false, false);
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
  #endif
}

DPtr<uint32_t> *nfd(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfdecompose(codepoints, false));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}

#ifndef UCS_NO_K
uint8_t nfkd_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw(InvalidCodepointException, SizeUnknownException) {
  #ifdef UCS_PLAY_DUMB
  return UCS_QC_YES;
  #else
  try {
    return nfqc(codepoints, pos, false, true);
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
  #endif
}

DPtr<uint32_t> *nfkd(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfdecompose(codepoints, true));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}
#endif

#ifndef UCS_NO_C

#ifndef UCS_PLAY_DUMB
vector<uint32_t> *nfcompose(const DPtr<uint32_t> *codepoints,
    const bool use_compat)
    THROWS(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  vector<uint32_t> *decomp = nfdecompose(codepoints, use_compat);
  if (decomp->size() == 0) {
    return decomp;
  }
  try {
    vector<uint32_t> *comp;
    try {
      comp = new vector<uint32_t>();
      comp->reserve(codepoints->size());
    } RETHROW_BAD_ALLOC
    comp->push_back(decomp->at(0));
    size_t i;
    for (i = 1; i < decomp->size(); i++) {
      uint32_t cp = decomp->at(i);
      uint8_t ccc = UCS_UNPACK_CCC(cp);
      cp = UCS_UNPACK_CODEPOINT(cp);
      bool starter_found = false;
      uint32_t starter;
      size_t starti = 0;
      signed long j;
      for (j = comp->size() - 1; j >= 0; j--) {
        uint32_t cp2 = comp->at(j);
        uint8_t ccc2 = UCS_UNPACK_CCC(cp2);
        cp2 = UCS_UNPACK_CODEPOINT(cp2);
        if (ccc2 == 0) {
          starter = cp2;
          starti = j;
          starter_found = true;
          break;
        }
        if (ccc2 >= ccc) { // blocked
          break;
        }
      }
      if (!starter_found) {
        comp->push_back(decomp->at(i));
        continue;
      }
      const uint64_t pair = (((uint64_t) starter) << 32) | ((uint64_t) cp);
      const uint64_t *lb = lower_bound(UCS_COMPOSITION_INDEX,
          UCS_COMPOSITION_INDEX + UCS_COMPOSITION_INDEX_LEN, pair);
      if (lb == UCS_COMPOSITION_INDEX + UCS_COMPOSITION_INDEX_LEN || *lb != pair) {
        comp->push_back(decomp->at(i));
        continue;
      }
      uint32_t offset = lb - UCS_COMPOSITION_INDEX;
      comp->at(starti) = UCS_COMPOSITIONS[offset];
    }
    delete decomp;
    return comp;
  } catch (BadAllocException &e) {
    delete decomp;
    RETHROW(e, "(rethrow)");
  }
}
TRACE(BadAllocException, "Couldn't allocate memory for UCS composition.")
#endif

uint8_t nfc_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw(InvalidCodepointException, SizeUnknownException) {
  #ifdef UCS_PLAY_DUMB
  return UCS_QC_YES;
  #else
  try {
    return nfqc(codepoints, pos, true, false);
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
  #endif
}

DPtr<uint32_t> *nfc(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfcompose(codepoints, false));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}

#ifndef UCS_NO_K
uint8_t nfkc_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw(InvalidCodepointException, SizeUnknownException) {
  #ifdef UCS_PLAY_DUMB
  return UCS_QC_YES;
  #else
  try {
    return nfqc(codepoints, pos, true, true);
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
  #endif
}

DPtr<uint32_t> *nfkc(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfcompose(codepoints, true));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}
#endif

#endif /* UCS_NO_C */

}
