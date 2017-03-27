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
 * @file ellis/core/sync_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_SYNC_INPUT_STREAM_HPP_
#define ELLIS_CORE_SYNC_INPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


/** Abstract API for synchronous (blocking) input streams.
 *
 * Note--a class may implement both sync_input_stream and sync_output_stream.
 */
class sync_input_stream {
public:
  /** Get another block of data from the stream, filling in buf and bytecount
   * for the block of data.  Buffer remains owned by the stream; caller
   * should not call next_input_buf() again until finished with the buffer.
   *
   * Return values:
   *   true  --> Data is available (and has been returned).
   *   false --> Data is unavailable (check extract_input_error() for details).
   */
  virtual bool next_input_buf(const byte **buf, size_t *bytecount) = 0;

  /** Put back data at the end of the block last returned from next_block(). */
  virtual void put_back(size_t bytecount) = 0;

  /** Return the error details.  Caller owns it now.  */
  virtual std::unique_ptr<err> extract_input_error() = 0;

  virtual ~sync_input_stream() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_SYNC_INPUT_STREAM_HPP_ */
