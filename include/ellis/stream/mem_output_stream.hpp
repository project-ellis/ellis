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


/*
 * @file ellis/stream/mem_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/sync_output_stream.hpp>

namespace ellis {


class mem_output_stream : public sync_output_stream {
  byte *m_buf;
  size_t m_len;
  size_t m_pos = 0;
  std::unique_ptr<err> m_err;

public:
  /**
   * Create a memory output stream that will fill in buf, up to len bytes,
   * always making sure to terminate the string after any given call to
   * emit().  This means that the actual maximum length of the string will
   * be one less than len, to allow for a null terminator.
   */
  mem_output_stream(void *buf, size_t len);

  bool next_output_buf(byte **buf, size_t *bytecount) override;
  bool emit(size_t bytecount) override;
  std::unique_ptr<err> extract_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_ */
