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


#include <ellis/stream/fd_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>
#include <unistd.h>

namespace ellis {


fd_output_stream::fd_output_stream(int fd) : m_fd(fd) {
}

bool fd_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  *buf = (byte *)m_buf;
  *bytecount = sizeof(m_buf);
  return true;
}

bool fd_output_stream::emit(size_t bytecount) {
  ELLIS_ASSERT_LTE(bytecount, sizeof(m_buf));
  size_t pos = 0;
  while (pos < bytecount) {
    int n = write(m_fd, m_buf, bytecount - pos);
    if (n == 0 || (n < 0 && errno == EINTR)) {
      continue;
    }
    else if (n < 0) {
      /* TODO: extract details via strerror?  Use errno? */
      m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "I/O error");
      return false;
    }
    /* We wrote some bytes. */
    pos += n;
  }
  return true;
}

unique_ptr<err> fd_output_stream::extract_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
