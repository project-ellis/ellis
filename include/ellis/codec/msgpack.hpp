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


enum class parse_state {
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


struct parse_ctx {
  parse_state state;
  uint_fast32_t data_len;
  union {
    uint_fast8_t header_len;
    uint64_t data;
  };
  /* Possible optimization: fold buf and (and maybe node too) into the union. */
  uint_fast32_t map_len;
  std::unique_ptr<std::vector<byte>> buf;
  std::unique_ptr<ellis::node> node;

  parse_ctx();
};


class msgpack_decoder : public decoder {
  std::vector<parse_ctx> m_parse_stack;

  node_progress handle_type(parse_ctx &ctx, byte b);

  void accum_str_header(parse_ctx & ctx, byte b);
  void accum_bin_header(parse_ctx & ctx, byte b);
  void accum_map_key_header(parse_ctx & ctx, byte b);
  node_progress accum_arr_header(parse_ctx & ctx, byte b);
  node_progress accum_map_header(parse_ctx & ctx, byte b);
  node_progress parse_byte(byte b);

public:
  msgpack_decoder();
  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;
  node_progress chop() override;
  void reset() override;
};


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
