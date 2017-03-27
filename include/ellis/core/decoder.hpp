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
 * @file ellis/core/decoder.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_DECODER_HPP_
#define ELLIS_CORE_DECODER_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/disposition.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


std::ostream & operator<<(std::ostream & os, const array_node & v);


/**
 * This abstract base class (interface) is used to decode/unpack/deserialize
 * JSON-encoded data from the provided buffers into a reconstructed in-memory
 * ellis node.
 */
class decoder {
public:
  /**
   * This function is called by the data stream to tell the decoder it may
   * decode up to bytecount bytes of data from the provided buffer.
   *
   * The callee should attempt to decode the given data to construct an ellis
   * node, and update the bytecount parameter to specify the number of bytes
   * that remain unused and available, if there are any.  Whatever bytes are
   * left will be reclaimed by the stream for use by the next consumer.
   *
   * If there has been a non-recoverable error in the decoding process, the
   * ERROR status will be returned (with details provided); otherwise, if an
   * ellis node has been completely decoded, a status of SUCCESS is returned
   * (with node provided); otherwise a status of CONTINUE will be returned (to
   * indicate that additional bytes must be provided via additional calls to
   * consume_buffer).
   *
   * If a status of SUCCESS or ERROR is returned, then the decoder must be
   * reset before any further calls to consume_buffer().
   */
  virtual node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) = 0;

  /**
   * Tell the decoder there are no more bytes coming.  The decoder will
   * decide whether a node can be created based on prior bytes, in which
   * case SUCCESS will be returned (with node provided), or whether
   * this would result in a truncated or malformed node, in which case
   * ERROR will be returned (with details provided).
   *
   * @return must be either SUCCESS (with value) or ERROR (with details).
   */
  virtual node_progress chop() = 0;

  /**
   * Reset the encoder to start encoding a new ellis node.
   *
   * This should result in the same behavior as constructing a new decoder,
   * but may be more efficient.
   */
  virtual void reset() = 0;

  virtual ~decoder() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_DECODER_HPP_ */
