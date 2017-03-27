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
 * @file ellis/codec/delimited_text.hpp
 *
 * @brief Ellis Delimited Text Codec C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_DELIMITED_TEXT_HPP_
#define ELLIS_CODEC_DELIMITED_TEXT_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/decoder.hpp>
#include <ellis/core/encoder.hpp>
#include <sstream>

namespace ellis {


class delimited_text_decoder : public decoder {
  std::unique_ptr<node> m_node;
  std::ostringstream m_ss;

  void _clear_ss();

public:
  // TODO: vector of delimiter strings (first level, second, ...)
  // e.g. delimited_text_decoder({"\n", " ", ","}) produces an array of arrays
  // of arrays.
  delimited_text_decoder();

  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;

  node_progress chop() override;

  void reset() override;
};


class delimited_text_encoder : public encoder {
  std::stringstream m_ss;
  size_t m_sspos = 0;
  size_t m_ssend = 0;

  void _clear_ss();

public:
  delimited_text_encoder();
  ~delimited_text_encoder();
  progress fill_buffer(
      byte *buf,
      size_t *bytecount) override;
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_DELIMITED_TEXT_HPP_ */
