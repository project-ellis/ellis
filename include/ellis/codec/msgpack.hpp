/*
 * @file ellis/codec/msgpack.hpp
 *
 * @brief Ellis msgpack C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_MSGPACK_HPP_
#define ELLIS_CODEC_MSGPACK_HPP_

#include <ellis/core/decoder.hpp>
#include <ellis/core/encoder.hpp>
#include <vector>

namespace ellis {


class msgpack_decoder : public decoder {
  std::vector<byte> m_buf;

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
