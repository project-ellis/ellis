/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <ellis/stream/file_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <ellis_private/using.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace ellis {


file_input_stream::file_input_stream(const char *filename) {
  if (strcmp(filename, "-") == 0) {
    m_fd = 0;
  } else {
    m_fd = open(filename, O_RDONLY);
  }
  if (m_fd < 0) {
    // TODO: map errno for more specifics
    THROW_ELLIS_ERR(IO, "bad pathname: " << filename);
  }
  m_fdis.reset(new fd_input_stream(m_fd));
}

file_input_stream::~file_input_stream() {
  m_fdis.reset();
  if (m_fd > 0) {
    close(m_fd);
    m_fd = -1;
  }
}

bool file_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  return m_fdis->next_input_buf(buf, bytecount);
}

void file_input_stream::put_back(size_t bytecount) {
  return m_fdis->put_back(bytecount);
}

unique_ptr<err> file_input_stream::extract_input_error() {
  return m_fdis->extract_input_error();
}


}  /* namespace ellis */
