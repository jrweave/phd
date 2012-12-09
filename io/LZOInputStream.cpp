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

#include "io/LZOInputStream.h"

#include <deque>
#include "ptr/MPtr.h"
#include "sys/endian.h"
#include "util/funcs.h"

namespace io {

using namespace sys;
using namespace util;

LZOInputStream::LZOInputStream(InputStream *is, deque<uint64_t> *index)
    throw(BaseException<void*>, TraceableException)
    : input_stream(is), index(index), buffer(NULL), offset(0), length(0),
      max_block_size(0), count(UINT64_C(0)), flags(UINT32_C(1)),
      header_read(false), no_header(false), ignore_checksum(false) {
  if (is == NULL) {
    THROW(BaseException<void*>, NULL, "is must not be NULL.");
  }
  if (lzo_init() != LZO_E_OK) {
    THROW(TraceableException, "Couldn't initialize LZO!");
  }
  this->checksum = lzo_adler32(0, NULL, 0);
}

LZOInputStream::LZOInputStream(InputStream *is, deque<uint64_t> *index,
    const bool no_header, const bool ignore_checksum)
    throw(BaseException<void*>, TraceableException)
    : input_stream(is), index(index), buffer(NULL), offset(0), length(0),
      max_block_size(0), count(UINT64_C(0)), flags(UINT32_C(1)),
      header_read(no_header), no_header(no_header),
      ignore_checksum(ignore_checksum) {
  if (is == NULL) {
    THROW(BaseException<void*>, NULL, "is must not be NULL.");
  }
  if (lzo_init() != LZO_E_OK) {
    THROW(TraceableException, "Couldn't initialize LZO!");
  }
  this->checksum = lzo_adler32(0, NULL, 0);
}

LZOInputStream::~LZOInputStream() THROWS(IOException) {
  DELETE(this->input_stream);
  if (this->buffer != NULL) {
    this->buffer->drop();
  }
  if (this->index != NULL) {
    DELETE(this->index);
  }
}
TRACE(IOException, "Problem deconstructing LZOInputStream.")

void LZOInputStream::close() THROWS(IOException) {
  if (this->index != NULL) {
    this->index->push_back(this->count);
  }
  this->input_stream->close();
}
TRACE(IOException, "Problem closing LZOInputStream.")

DPtr<uint8_t> *LZOInputStream::read() THROWS(IOException, BadAllocException) {
  // in case there are leftovers from calling read(const int64_t)
  if (this->offset < this->length) {
    if (this->offset == 0 && this->length == this->buffer->size()) {
      this->buffer->hold();
      return this->buffer;
    }
    DPtr<uint8_t> *p = this->buffer->sub(this->offset,
                                         this->length - this->offset);
    this->offset = this->length;
    return p;
  }
  uint32_t compressed_size, uncompressed_size;
  if (!this->readu32(uncompressed_size)) {
    return NULL;
  }
  if (uncompressed_size == 0) {
    this->readFooter();
    return NULL;
  }
  if (!this->readu32(compressed_size)) {
    THROW(IOException, "Unexpected end of file.");
  }
  if (!this->header_read) {
    this->header_read = true;
    // HARDCODED MAGIC HERE
    if (uncompressed_size == UINT32_C(0x00e94c5a) &&
        (compressed_size & UINT32_C(0xffffff00)) == UINT32_C(0x4fff1a00)) {
      uint8_t first_flag_byte = (uint8_t) (compressed_size & UINT32_C(0x0ff));
      this->readHeader(first_flag_byte);
      if (!this->readu32(uncompressed_size)) {
        return NULL;
      }
      if (uncompressed_size == 0) {
        this->readFooter();
        return NULL;
      }
      if (!this->readu32(compressed_size)) {
        THROW(IOException, "Unexpected end of file.");
      }
    }
  }
  if (this->max_block_size > 0) {
    if (compressed_size > this->max_block_size
        || uncompressed_size > this->max_block_size) {
      THROW(IOException, "Data appears to be corrupted.");
    }
  }
  if (compressed_size <= 0 || compressed_size > uncompressed_size) {
    THROW(IOException, "Data appears to be corrupted.");
  }
  if (!this->readBlock(compressed_size, uncompressed_size) ||
      this->length - this->offset < compressed_size) {
    THROW(IOException, "Unexpected end of file.");
  }
  if (this->length - this->offset != compressed_size) {
    THROW(IOException, "Sanity check failed.  Internal error.");
  }
  if (compressed_size < uncompressed_size) {
    lzo_uint new_size = (lzo_uint) uncompressed_size;
    int ok = lzo1x_decompress_safe(
                this->buffer->dptr() + this->offset,
                compressed_size,
                this->buffer->dptr(),
                &new_size, NULL);
    if (ok != LZO_E_OK) {
      THROW(IOException, "Something went wrong in LZO decompression.");
    }
    if (new_size != uncompressed_size) {
      THROW(IOException, "Uncompressed block size is the expected size.");
    }
    this->offset = 0;
    this->length = new_size;
  }
  if (!this->ignore_checksum && (this->flags & 1)) {
    this->checksum = lzo_adler32(this->checksum,
                                 this->buffer->dptr() + this->offset,
                                 uncompressed_size);
  }
  if (this->index != NULL) {
    this->index->push_back(this->count);
    this->count += compressed_size + (sizeof(uint32_t) << 1);
  }
  if (this->offset == 0 && this->length == this->buffer->size()) {
    this->buffer->hold();
    return this->buffer;
  }
  DPtr<uint8_t> *p = this->buffer->sub(this->offset,
                                       this->length - this->offset);
  this->offset = this->length;
  return p;
}
TRACE(IOException, "Problem reading from LZOInputStream.")

DPtr<uint8_t> *LZOInputStream::read(const int64_t amount)
    THROWS(IOException, BadAllocException) {
  DPtr<uint8_t> *p = this->read();
  if (p == NULL || p->size() <= amount) {
    return p;
  }
  DPtr<uint8_t> *p2 = p->sub(0, amount);
  p->drop();
  this->offset -= amount;
  return p2;
}
TRACE(IOException, "Problem reading from LZOInputStream.")

void LZOInputStream::reset() THROWS(IOException) {
  this->input_stream->reset();
  this->offset = 0;
  this->length = 0;
  this->count = 0;
  this->max_block_size = 0;
  this->flags = UINT32_C(1);
  this->checksum = lzo_adler32(0, NULL, 0);
  this->header_read = this->no_header;
}
TRACE(IOException, "Problem resetting LZOInputStream.")

// readHeader is a bit of a misnomer.  First eight bytes are already
// read by the time readHeader is called, so readRestOfHeader would
// be more appropriate... but I hate the way that method name looks.
void LZOInputStream::readHeader(const uint8_t first_flag_byte)
    THROWS(IOException) {
  if (first_flag_byte != UINT8_C(0)) {
    THROW(IOException, "Unrecognized flags in LZO header.");
  }
  size_t len = 3 + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t);
  if (!this->readBlock(len)) {
    THROW(IOException, "Problem reading header.");
  }
  const uint8_t *p = this->buffer->dptr() + this->offset;
  if (p[0] != UINT8_C(0) || p[1] != UINT8_C(0) || p[2] > UINT8_C(1)) {
    THROW(IOException, "Unrecognized flags in LZO header.");
  }
  this->flags = p[2];
  if (p[3] != UINT8_C(1)) {
    THROW(IOException, "Unsupported method specified in LZO header.");
  }
  if (p[4] != UINT8_C(1)) {
    THROW(IOException,
          "Unsupported compression level specified in LZO header.");
  }
  p += 5;
  uint32_t u32;
  memcpy(&u32, p, sizeof(uint32_t));
  if (is_little_endian()) {
    reverse_bytes(u32);
  } else if (!is_big_endian()) {
    THROW(IOException,
          "Strange endianness (neither big nor little) is unsupported.");
  }
  this->max_block_size = (lzo_uint)u32;
  len = u32 + (u32 >> 4) + 64 + 3;
  if (this->buffer != NULL) { // this will always be true, but just in case
    this->buffer->drop();
  }
  try {
    NEW(this->buffer, MPtr<uint8_t>, len);
  } catch (BadAllocException &e) {
    THROW(IOException, e.what());
  }
  this->offset = 0;
  this->length = 0;
  this->count = 17; // TODO not good to have so many integer constants
}
TRACE(IOException, "Problem reading LZO header.")

bool LZOInputStream::readu32(uint32_t &num) THROWS(IOException) {
  if (!this->readBlock(sizeof(uint32_t))) {
    if (this->offset != this->length) {
      THROW(IOException, "Expected 32-bit integer but found other bytes.");
    }
    return false;
  }
  memcpy(&num, this->buffer->dptr() + this->offset, sizeof(uint32_t));
  this->offset = this->length;
  if (is_little_endian()) {
    reverse_bytes(num);
  } else if (!is_big_endian()) {
    THROW(IOException,
          "Strange endianness (neither big nor little) is unsupported.");
  }
  return true;
}
TRACE(IOException, "Problem reading integer in LZOInputStream.")

// Reads len bytes and places it in the last len bytes of this->buffer.
// Returns false if unable to read len bytes.
bool LZOInputStream::readBlock(const uint32_t len) THROWS(IOException) {
  return this->readBlock(len, len);
}
TRACE(IOException, "Trouble reading block in LZOInputStream.")

bool LZOInputStream::readBlock(const uint32_t len, const uint32_t maxlen)
    THROWS(IOException) {
  if (this->offset != this->length) {
    THROW(IOException, "Failed sanity check.");
  }
  if (this->max_block_size > 0) {
    if (maxlen > this->max_block_size) {
      THROW(IOException, "Block requested larger than max block size found in header.  Possible LZO data corruption.");
    }
  } else {
    size_t needed_len = maxlen + (maxlen >> 4) + 64 + 3;
    if (this->buffer == NULL || this->buffer->size() < needed_len) {
      if (this->buffer != NULL) {
        this->buffer->drop();
      }
      try {
        NEW(this->buffer, MPtr<uint8_t>, needed_len);
      } catch (BadAllocException &e) {
        THROW(IOException, e.what());
      }
    }
  }
  if (!this->buffer->alone()) {
    DPtr<uint8_t> *newbuf = NULL;
    try {
      NEW(newbuf, MPtr<uint8_t>, this->buffer->size());
    } catch (BadAllocException &e) {
      THROW(IOException, e.what());
    }
    this->buffer->drop();
    this->buffer = newbuf;
  }
  this->offset = this->buffer->size() - len;
  uint8_t *write_to = this->buffer->dptr() + this->offset;
  const uint8_t *end = write_to + len;
  DPtr<uint8_t> *p = this->input_stream->read(len);
  while (p != NULL) {
    memcpy(write_to, p->dptr(), p->size());
    write_to += p->size();
    p->drop();
    if (write_to == end) {
      break;
    }
    p = this->input_stream->read(end - write_to);
  }
  this->length = write_to - this->buffer->dptr();
  return write_to == end;
}
TRACE(IOException, "Problem reading block in LZOInputStream.")

// readFooter is a bit of a misnomer.  First byte (EOF marker) is already
// read by the time readFooter is called, so readRestOfFooter would
// be more appropriate... but I hate the way that method name looks.
void LZOInputStream::readFooter() THROWS(IOException) {
  uint32_t chsum;
  if (this->readu32(chsum)) {
    if (!this->ignore_checksum && this->checksum != chsum) {
      THROW(IOException, "Checksums do not match!  LZO data corrupted!");
    }
  }
  DPtr<uint8_t> *p = this->input_stream->read();
  if (p != NULL) {
    p->drop();
    THROW(IOException, "Extra bytes found after LZO footer.  Possible data corruption.");
  }
}
TRACE(IOException, "Problem reading footer in LZOInputStream.")

}
