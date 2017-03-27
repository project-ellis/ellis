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


#include <ellis/stream/mem_output_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


mem_output_stream::mem_output_stream(void *buf, size_t len) :
  m_buf((byte *)buf), m_len(len)
{
  ELLIS_ASSERT_GT(m_len, 0);
}

bool mem_output_stream::next_output_buf(byte **buf, size_t *bytecount) {
  if (m_pos < m_len - 1) {
    /* We have some leftover buffer from earlier.  Return that. */
    *buf = m_buf + m_pos;
    *bytecount = m_len - 1 - m_pos;
    return true;
  } else {
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "end of memory stream reached");
    return false;
  }
}

bool mem_output_stream::emit(size_t bytecount) {
  m_pos += bytecount;
  ELLIS_ASSERT_LT(m_pos, m_len);
  m_buf[m_pos] = '\0';

  return true;
}

unique_ptr<err> mem_output_stream::extract_output_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
