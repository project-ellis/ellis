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


#include <ellis/stream/file_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <ellis_private/using.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace ellis {


file_output_stream::file_output_stream(const char *filename) {
  if (strcmp(filename, "-") == 0) {
    m_fd = 1;
  } else {
    m_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  }
  if (m_fd < 0) {
    // TODO: map errno for more specifics
    THROW_ELLIS_ERR(IO, "bad pathname: " << filename);
  }
  m_fdos.reset(new fd_output_stream(m_fd));
}

file_output_stream::~file_output_stream() {
  m_fdos.reset();
  if (m_fd > 0) {
    close(m_fd);
    m_fd = -1;
  }
}

bool file_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  return m_fdos->next_output_buf(buf, bytecount);
}

bool file_output_stream::emit(size_t bytecount) {
  return m_fdos->emit(bytecount);
}

unique_ptr<err> file_output_stream::extract_output_error() {
  return m_fdos->extract_output_error();
}


}  /* namespace ellis */
