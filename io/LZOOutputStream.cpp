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

#include "io/LZOOutputStream.h"

#include "ptr/MPtr.h"
#include "sys/endian.h"
#include "util/funcs.h"

namespace io {

using namespace sys;
using namespace util;

static const uint8_t LZOMAGIC[7] = { UINT8_C(0x00),
    UINT8_C(0xe9), UINT8_C(0x4c), UINT8_C(0x5a),
    UINT8_C(0x4f), UINT8_C(0xff), UINT8_C(0x1a) };

LZOOutputStream::LZOOutputStream(OutputStream *os, deque<uint64_t> *index,
    const size_t max_block_size, const bool write_header,
    const bool write_footer, const bool do_checksum)
    throw(BaseException<void*>, BadAllocException, TraceableException)
    : output_stream(os), index(index), count(0),
      max_block_size(max_block_size), flags(do_checksum ? 1 : 0),
      write_header(write_header), write_footer(write_footer),
      header_written(false) {
  if (os == NULL) {
    THROW(BaseException<void*>, NULL, "os must not be NULL.");
  }
  if (lzo_init() != LZO_E_OK) {
    THROW(TraceableException, "Unable to initialize LZO!");
  }
  size_t outsize = max_block_size + (max_block_size >> 4) + 67
                   + (sizeof(uint32_t) << 1);
  try {
    NEW(this->output, MPtr<uint8_t>, outsize);
  } RETHROW_BAD_ALLOC
  try {
    NEW(this->work_memory, MPtr<uint8_t>, LZO1X_1_MEM_COMPRESS);
  } RETHROW_BAD_ALLOC
  if (this->flags & 1) {
    this->checksum = lzo_adler32(0, NULL, 0);
  }
}

LZOOutputStream::~LZOOutputStream() THROWS(IOException) {
  DELETE(this->output_stream);
  if (this->index != NULL) {
    DELETE(this->index);
  }
  this->work_memory->drop();
  this->output->drop();
}
TRACE(IOException, "Trouble deconstructing LZOOutputStream.")

deque<uint64_t> *LZOOutputStream::getIndex() throw() {
  return this->index;
}

void LZOOutputStream::close() THROWS(IOException) {
  if (this->write_header && !this->header_written) {
    this->writeHeader();
    this->header_written = true;
  }
  if (this->write_footer) {
    this->writeFooter();
  }
  this->output_stream->close();
}
TRACE(IOException, "Trouble closing LZOOutputStream.")

void LZOOutputStream::flush() THROWS(IOException) {
  this->output_stream->flush();
}
TRACE(IOException, "Trouble flushing LZOOutputStream.")

void LZOOutputStream::write(DPtr<uint8_t> *buf, size_t &nwritten)
    THROWS(IOException, SizeUnknownException, BaseException<void*>) {
  if (buf == NULL) {
    THROW(BaseException<void*>, NULL, "buf must not be NULL.");
  }
  if (!buf->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  if (this->write_header && !this->header_written) {
    this->writeHeader();
    this->header_written = true;
  }
  if (buf->size() > this->max_block_size) {
    THROW(IOException, "Buffer to be written exceeds specified maximum block size.");
  }
  if (!this->output->alone()) {
    size_t outsize = this->output->size();
    DPtr<uint8_t> *p = NULL;
    try {
      NEW(p, MPtr<uint8_t>, outsize);
    } catch (BadAllocException &e) {
      THROW(IOException, e.what());
    }
    this->output->drop();
    this->output = p;
  }
  if (this->flags & 1) {
    this->checksum = lzo_adler32(this->checksum, buf->dptr(), buf->size());
  }
  uint32_t uncompressed_size = (uint32_t) buf->size();
  size_t compressed_size = this->output->size();
  uint8_t *outp = this->output->dptr() + (sizeof(uint32_t) << 1);
  int ok = lzo1x_1_compress(buf->dptr(), uncompressed_size, outp,
                            &compressed_size, this->work_memory->dptr());
  if (ok != LZO_E_OK || compressed_size > uncompressed_size + (uncompressed_size >> 4) + 67) {
    THROW(IOException, "Problem performing LZO compression.");
  }
  uint32_t sz = uncompressed_size;
  if (is_little_endian()) {
    reverse_bytes(sz);
  } else if (!is_big_endian()) {
    THROW(IOException, "Unhandled endianness, neither big nor little.");
  }
  outp = this->output->dptr();
  memcpy(outp, &sz, sizeof(uint32_t));
  outp += sizeof(uint32_t);
  size_t len = (sizeof(uint32_t) << 1);
  if (compressed_size < uncompressed_size) {
    len += compressed_size;
    sz = (uint32_t)compressed_size;
    if (is_little_endian()) {
      reverse_bytes(sz);
    } else if (!is_big_endian()) {
      THROW(IOException, "Unhandled endianness, neither big nor little.");
    }
    memcpy(outp, &sz, sizeof(uint32_t));
  } else {
    len += uncompressed_size;
    memcpy(outp, &sz, sizeof(uint32_t));
    outp += sizeof(uint32_t);
    memcpy(outp, buf->dptr(), buf->size());
  }
  DPtr<uint8_t> *p = this->output->sub(0, len);
  this->output_stream->write(p);
  if (this->index != NULL) {
    this->index->push_back(this->count);
    this->count += p->size();
  }
  p->drop();
}
TRACE(IOException, "Trouble writing in LZOOutputStream.")

void LZOOutputStream::writeHeader() THROWS(IOException) {
  size_t len = 7 + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t)
                 + sizeof(uint32_t);
  this->count += len;
  DPtr<uint8_t> *p;
  try {
    NEW(p, MPtr<uint8_t>, len);
  } catch (BadAllocException &e) {
    THROW(IOException, e.what());
  }
  uint8_t *write_to = p->dptr();
  memcpy(write_to, LZOMAGIC, 7);
  write_to += 7;
  uint32_t u32 = this->flags;
  if (is_little_endian()) {
    reverse_bytes(u32);
  } else if (!is_big_endian()) {
    THROW(IOException, "Unhandled endianness, neither big nor little.");
  }
  memcpy(write_to, &u32, sizeof(uint32_t));
  write_to += sizeof(uint32_t);
  *write_to = UINT8_C(1);
  ++write_to;
  *write_to = UINT8_C(1);
  ++write_to;
  u32 = this->max_block_size;
  if (is_little_endian()) {
    reverse_bytes(u32);
  } else if (!is_big_endian()) {
    THROW(IOException, "Unhandled endianness, neither big nor little.");
  }
  memcpy(write_to, &u32, sizeof(uint32_t));
  this->output_stream->write(p);
  p->drop();
}
TRACE(IOException, "Trouble writing header in LZOOutputStream.")

void LZOOutputStream::writeFooter() THROWS(IOException) {
  size_t len = sizeof(uint32_t);
  if (this->flags & 1) {
    len += sizeof(uint32_t);
  }
  DPtr<uint8_t> *p;
  try {
    NEW(p, MPtr<uint8_t>, len);
  } catch (BadAllocException &e) {
    THROW(IOException, e.what());
  }
  uint8_t *write_to = p->dptr();
  uint32_t u32 = UINT32_C(0);
  memcpy(write_to, &u32, sizeof(uint32_t));
  if (this->flags & 1) {
    write_to += sizeof(uint32_t);
    u32 = this->checksum;
    if (is_little_endian()) {
      reverse_bytes(u32);
    } else if (!is_big_endian()) {
      THROW(IOException, "Unhandled endianness, neither big nor little.");
    }
    memcpy(write_to, &u32, sizeof(uint32_t));
  }
  this->output_stream->write(p);
  p->drop();
}
TRACE(IOException, "Trouble writing footer in LZOOutputStream.")

}
