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

#include "par/MPIFileOutputStream.h"

namespace par {

inline
MPIFileOutputStream::~MPIFileOutputStream() throw(IOException) {
  this->buffer->drop();
}

inline
void MPIFileOutputStream::close() throw(IOException) {
  try {
    this->file.Close();
  } catch (MPI::Exception &e) {
    THROW(IOException, e.Get_error_string());
  }
}

inline
void MPIFileOutputStream::flush() THROWS(IOException) {
  if (this->length > 0) {
    this->writeBuffer();
    this->length = 0;
  }
}
TRACE(IOException, "Error when flushing.")

}
