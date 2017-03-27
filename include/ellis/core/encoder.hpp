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
 * @file ellis/core/encoder.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_ENCODER_HPP_
#define ELLIS_CORE_ENCODER_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/disposition.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const array_node & v);


/**
 * This abstract base class (interface) is used to encode/pack/serialize
 * JSON-encoded data from an in-memory ellis node into the provided buffers.
 */
class encoder {
public:
  /**
   * This function is called by the data stream to tell the encoder it may
   * encode up to bytecount bytes of data into the provided buffer.
   *
   * The callee should use up to the given size, as needed, for encoding, and
   * update bytecount to reflect the amount of unused buffer space left over.
   *
   * If there has been a non-recoverable error in the encoding process, the
   * ERROR status will be returned and the details provided; otherwise, if an
   * ellis node has been completely encoded, a status of SUCCESS is returned;
   * otherwise a status of CONTINUE will be returned (to indicate that
   * additional space is needed for the encoding--to be provided via
   * additional calls to fill_buffer).
   *
   * If a status of SUCCESS or ERROR is returned, the encoder must be reset
   * before any further calls to fill_buffer().
   */
  virtual progress fill_buffer(
      byte *buf,
      size_t *bytecount) = 0;

  /**
   * Reset the encoder to start encoding new_node into output buffers.
   */
  virtual void reset(const node *new_node) = 0;

  virtual ~encoder() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_ENCODER_HPP_ */
