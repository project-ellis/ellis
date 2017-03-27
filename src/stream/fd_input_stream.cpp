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


#include <ellis/stream/fd_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <unistd.h>

namespace ellis {

fd_input_stream::fd_input_stream(int fd) : m_fd(fd) {
}

bool fd_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  int n = 0;
  if (m_pos < m_avail) {
    /* We have some leftover buffer from earlier.  Return that. */
    goto give_buffer;
  }

  /* No more bytes in current block?  Then try to get another one. */
  m_pos = 0;
  m_avail = 0;
  while (1) {
    n = read(m_fd, m_buf, sizeof(m_buf));
    if (n < 0 && errno == EINTR) {
      continue;
    }
    else {
      break;
    }
  }
  if (n < 0) {
    /* TODO: extract details via strerror?  Use errno? */
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "I/O error");
    return false;
  }
  else if (n == 0) {
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "end of file");
    return false;
  }
  /* Got some data. */
  m_avail = n;

give_buffer:
  *buf = (const byte *)(m_buf + m_pos);
  *bytecount = m_avail - m_pos;
  /* Treat input as consumed unless put_back is called. */
  m_pos = m_avail;
  return true;
}

void fd_input_stream::put_back(size_t bytecount) {
  m_pos = m_avail - bytecount;
}

unique_ptr<err> fd_input_stream::extract_input_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
