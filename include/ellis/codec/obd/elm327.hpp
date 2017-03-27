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
 * @file codec/elm327.hpp
 *
 * @brief Ellis ELM327 codec C++ header.
 *
 * This decoder implements decoding of ECU responses in the OBD II protocol over
 * ELM327. Specifically, it currently implements only OBD II modes 01 and 02 (SAE
 * Standard modes) and does not implement anything vehicle-specific.
 */

#pragma once
#ifndef ELLIS_CODEC_ELM327_HPP_
#define ELLIS_CODEC_ELM327_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/decoder.hpp>

namespace ellis {
namespace obd {

class elm327_decoder : public decoder {
  std::unique_ptr<node> m_node;

  /** Makes an OBD II node from a given byte sequence.
   *
   * @param start a pointer to the start of the byte sequence
   * @param length the length of the byte sequence, in bytes
   *
   * @return a new node representing an OBD II datum
   * */
  static node make_obd_node(const byte *start, size_t bytecount);

public:
  /** Constructor. */
  elm327_decoder();

  /** Consumes a given buffer of data coming from an ELM327 OBD II reader. Each
   * buffer represents an ECU response to a query.
   *
   * For more details regarding the OBD II, see the standard or:
   *
   * https://en.wikipedia.org/wiki/OBD-II_PIDs
   *
   * In particular:
   * https://en.wikipedia.org/wiki/OBD-II_PIDs#Response
   *
   * For more information regarding the ELM327 device, see:
   * https://en.wikipedia.org/wiki/ELM327
   * or more official sources :).
   */
  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;

  node_progress chop() override;

  void reset() override;
};


}  /* namespace obd */
}  /* namespace ellis */

#endif  /* ELLIS_CODEC_ELM327_HPP_ */
