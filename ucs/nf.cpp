#include "ucs/nf.h"

#include <algorithm>
#include <new>
#include <sstream>
#include <vector>
#include "ptr/MPtr.h"

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
DPtr<uint32_t> *nfreturn(DPtr<uint32_t> *p) {
  size_t i;
  for (i = 0; i < p->size(); i++) {
    (*p)[i] = UCS_UNPACK_CODEPOINT((*p)[i]);
  }
  return p;
}
#endif /* UCS_PLAY_DUMB */

#ifndef UCS_PLAY_DUMB
DPtr<uint32_t> *nfdecompose(const DPtr<uint32_t> *codepoints,
    const bool use_compat)
    THROWS (InvalidCodepointException, SizeUnknownException,
    BadAllocException, bad_alloc) {
  if (!codepoints->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  size_t i;
  size_t max = codepoints->size();
  const DPtr<uint32_t> &cps = *codepoints;
  vector<uint32_t> decomp;
  decomp.reserve(codepoints->size());
  for (i = 0; i < max; i++) {
    const uint32_t *d = nflookupd(cps[i]);
    if (d == NULL) {
      decomp.push_back(cps[i]);
      continue;
    }
    if (use_compat) {
      const uint32_t len = UCS_DECOMP_COMPAT_LEN(d);
      const uint32_t *c = UCS_DECOMP_COMPAT_CHARS(d);
      decomp.insert(decomp.end(), c, c + len);
    } else {
      const uint32_t len = UCS_DECOMP_CANON_LEN(d);
      const uint32_t *c = UCS_DECOMP_CANON_CHARS(d);
      decomp.insert(decomp.end(), c, c + len);
    }
  }
  vector<uint32_t>::iterator it;
  for (it = decomp.begin(); it != decomp.end(); it++) {
    if (UCS_UNPACK_CCC(*it) > 0) {
      vector<uint32_t>::iterator start = it;
      for (; it != decomp.end() && UCS_UNPACK_CCC(*it) > 0; it++) {
        // do nothing
      }
      // since combining class is in upper 8 bits,
      // this will sort by combining class
      stable_sort(start, it, nfcmpccc);
      it--;
    }
  }
  DPtr<uint32_t> *p = new MPtr<uint32_t>(decomp.size());
  for (i = 0; i < decomp.size(); i++) {
    (*p)[i] = decomp[i];
  }
  return p;
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
      if (pos != NULL && result == UCS_QC_YES) {
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
      if (pos != NULL && result == UCS_QC_YES) {
        *pos = i;
      }
      return UCS_QC_NO;
    }
    if (check == UCS_QC_MAYBE) {
      if (pos != NULL && result == UCS_QC_YES) {
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

#ifndef UCS_PLAY_DUMB
#ifndef UCS_NO_C
DPtr<uint32_t> *nfcompose(const DPtr<uint32_t> *codepoints, const bool compat)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException);
#endif

DPtr<uint32_t> *nfopt(DPtr<uint32_t> *codepoints, bool use_c, bool use_k)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  try {
    size_t n;
    uint8_t qc;
    qc = nfqc(codepoints, &n, use_c, use_k);
    if (qc == UCS_QC_YES) {
      codepoints->hold();
      return codepoints;
    }
    DPtr<uint32_t> *normed;
    #ifdef UCS_NO_C
    normed = nfdecompose(codepoints, use_k);
    #else
    if (use_c) {
      normed = nfcompose(codepoints, use_k);
    } else {
      normed = nfdecompose(codepoints, use_k);
    }
    #endif
    return normed;
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
}
#endif

DPtr<uint32_t> *nfd_opt(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfopt(codepoints, false, false));
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

DPtr<uint32_t> *nfkd_opt(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfopt(codepoints, false, true));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}
#endif

#ifndef UCS_NO_C

#ifndef UCS_PLAY_DUMB
DPtr<uint32_t> *nfcompose(const DPtr<uint32_t> *codepoints,
    const bool use_compat)
    THROWS(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  DPtr<uint32_t> *comp = nfdecompose(codepoints, use_compat);
  if (comp->size() == 0) {
    return comp;
  }
  try {
    size_t i;
    size_t newsize = 1;
    for (i = 1; i < comp->size(); i++) {
      uint32_t cp = (*comp)[i];
      uint8_t ccc = UCS_UNPACK_CCC(cp);
      cp = UCS_UNPACK_CODEPOINT(cp);
      bool starter_found = false;
      uint32_t starter;
      size_t starti = 0;
      signed long j;
      for (j = newsize - 1; j >= 0; j--) {
        uint32_t cp2 = (*comp)[j];
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
        (*comp)[newsize] = (*comp)[i];
        newsize++;
        continue;
      }
      const uint64_t pair = (((uint64_t) starter) << 32) | ((uint64_t) cp);
      const uint64_t *lb = lower_bound(UCS_COMPOSITION_INDEX,
          UCS_COMPOSITION_INDEX + UCS_COMPOSITION_INDEX_LEN, pair);
      if (lb == UCS_COMPOSITION_INDEX + UCS_COMPOSITION_INDEX_LEN || *lb != pair) {
        (*comp)[newsize] = (*comp)[i];
        newsize++;
        continue;
      }
      uint32_t offset = lb - UCS_COMPOSITION_INDEX;
      (*comp)[starti] = UCS_COMPOSITIONS[offset];
    }
    DPtr<uint32_t> *retcomp = new MPtr<uint32_t>(newsize);
    memcpy(retcomp->ptr(), comp->ptr(), newsize * sizeof(uint32_t));
    comp->drop();
    return retcomp;
  } catch (BadAllocException &e) {
    comp->drop();
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

DPtr<uint32_t> *nfc_opt(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfopt(codepoints, true, false));
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

DPtr<uint32_t> *nfkc_opt(DPtr<uint32_t> *codepoints)
    throw(InvalidCodepointException, SizeUnknownException,
    BadAllocException) {
  #ifdef UCS_PLAY_DUMB
  codepoints->hold();
  return codepoints;
  #else
  try {
    return nfreturn(nfopt(codepoints, true, true));
  } JUST_RETHROW(InvalidCodepointException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BadAllocException, "(rethrow)")
  #endif
}
#endif

#endif /* UCS_NO_C */

}
