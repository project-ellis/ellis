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

namespace ellis {


class msgpack_decoder : public decoder {

public:
  msgpack_decoder();
  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;
  node_progress chop() override;
  void reset() override;
};


class msgpack_encoder : public encoder {
public:
  msgpack_encoder();
  progress fill_buffer(
      byte *buf,
      size_t *bytecount) override;
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_MSGPACK_HPP_ */
