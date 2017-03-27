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
 * @file ellis/codec/msgpack.hpp
 *
 * @brief Ellis msgpack C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_MSGPACK_HPP_
#define ELLIS_CODEC_MSGPACK_HPP_

#include <ellis/core/decoder.hpp>
#include <ellis/core/defs.hpp>
#include <ellis/core/encoder.hpp>
#include <vector>

namespace ellis {


enum class msgpack_parse_state {
  UNDEFINED,
  COMPLETE,
  ARRAY_HEADER,
  BIN_HEADER,
  MAP_HEADER,
  STR_HEADER,
  ARRAY_DATA,
  BIN_DATA,
  STR_DATA,
  MAP_KEY_TYPE,
  MAP_KEY_HEADER,
  MAP_KEY_DATA,
  MAP_VALUE_DATA,
  FLOAT32_DATA,
  FLOAT64_DATA,
  INT16_DATA,
  INT32_DATA,
  INT64_DATA,
  INT8_DATA,
  UINT16_DATA,
  UINT32_DATA,
  UINT8_DATA,
};


/** All the state needed to represent the current msgpack parse state. */
struct msgpack_parse_ctx {
  /** The current parse state; used to interpret incoming parse data. */
  msgpack_parse_state state;
  /** The length of the data currently being parsed. */
  uint_fast32_t data_len;
  union {
    /** Header length, if currently parsing a header. Otherwise unused. */
    uint_fast8_t header_len;
    /** The current data, for numeric types. */
    uint64_t data;
  };
  /* Possible optimization: fold buf and (and maybe node too) into the union. */

  /** The length of a map, if parsing a map. Otherwise unused. */
  uint_fast32_t map_len;
  /** A buffer used to store vector data (e.g. strings or binary). */
  std::unique_ptr<std::vector<byte>> buf;
  /** The node output of the current parse. */
  std::unique_ptr<ellis::node> node;

  msgpack_parse_ctx();
};


/** A msgpack decoder. */
class msgpack_decoder : public decoder {
  std::vector<msgpack_parse_ctx> m_parse_stack;

  node_progress handle_type(msgpack_parse_ctx &ctx, byte b);

  void accum_str_header(msgpack_parse_ctx & ctx, byte b);
  void accum_bin_header(msgpack_parse_ctx & ctx, byte b);
  void accum_map_key_header(msgpack_parse_ctx & ctx, byte b);
  node_progress accum_arr_header(msgpack_parse_ctx & ctx, byte b);
  node_progress accum_map_header(msgpack_parse_ctx & ctx, byte b);
  node_progress parse_byte(byte b);

public:
  msgpack_decoder();
  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;
  node_progress chop() override;
  void reset() override;
};


/** A msgpack encoder. */
class msgpack_encoder : public encoder {
  std::vector<byte> m_buf;
  size_t m_pos;
  size_t m_end;

  void _clear_buf();
  void _buf_out(const node &n);
  void _buf_out_str(const char *s, size_t len);

  void _push_be(uint8_t val);
  void _push_be(uint16_t val);
  void _push_be(uint32_t val);
  void _push_be(uint64_t val);

public:
  msgpack_encoder();
  progress fill_buffer(
      byte *buf,
      size_t *bytecount) override;
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_MSGPACK_HPP_ */
