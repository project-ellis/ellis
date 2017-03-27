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
 * @file ellis/stream/file_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/sync_output_stream.hpp>

namespace ellis {


/* forward declare */
class fd_output_stream;

class file_output_stream : public sync_output_stream {
  std::unique_ptr<fd_output_stream> m_fdos;
  int m_fd = -1;

public:
  file_output_stream(const char *filename);

  ~file_output_stream();

  bool next_output_buf(byte **buf, size_t *bytecount) override;

  bool emit(size_t bytecount) override;

  std::unique_ptr<err> extract_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_ */
