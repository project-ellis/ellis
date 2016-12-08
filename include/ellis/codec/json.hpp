/*
 * @file codec/json.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_JSON_HPP_
#define ELLIS_CODEC_JSON_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/stream_decoder.hpp>
#include <ellis/stream_encoder.hpp>
#include <memory>

namespace ellis {


class json_decoder : public stream_decoder {
public:
  json_decoder();
  decoding_status consume_buffer(
      const byte *buf,
      int *bytecount) override;
  std::unique_ptr<node> get_node() override;
  std::unique_ptr<err> get_error() override;
  void reset() override;
};


class json_encoder : public stream_encoder {
public:
  json_encoder();
  encoding_status fill_buffer(
      byte *buf,
      int *bytecount) override;
  std::unique_ptr<err> get_error() override;
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_JSON_HPP_ */