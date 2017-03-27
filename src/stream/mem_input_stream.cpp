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


#include <ellis/stream/mem_input_stream.hpp>

#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


mem_input_stream::mem_input_stream(const void *buf, size_t len) :
  m_buf((const byte *)buf), m_len(len)
{
}

bool mem_input_stream::next_input_buf(const byte **buf, size_t *bytecount) {
  if (m_pos < m_len) {
    /* We have some leftover buffer from earlier.  Return that. */
    *buf = m_buf + m_pos;
    *bytecount = m_len - m_pos;
    /* Treat input as consumed unless put_back is called. */
    m_pos = m_len;
    return true;
  } else {
    m_err = MAKE_UNIQUE_ELLIS_ERR(IO, "end of memory stream reached");
    return false;
  }
}

void mem_input_stream::put_back(size_t bytecount) {
  m_pos = m_len - bytecount;
}

unique_ptr<err> mem_input_stream::extract_input_error() {
  return std::move(m_err);
}


}  /* namespace ellis */
