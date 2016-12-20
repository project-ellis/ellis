/*
 * @file ellis/codec/json.hpp
 *
 * @brief Ellis JSON codec C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_JSON_HPP_
#define ELLIS_CODEC_JSON_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/stream_decoder.hpp>
#include <ellis/core/stream_encoder.hpp>
#include <sstream>

namespace ellis {


class json_parser;
class json_tokenizer;

class json_decoder : public stream_decoder {

  std::unique_ptr<json_tokenizer> m_toker;
  std::unique_ptr<json_parser> m_parser;
  std::unique_ptr<err> m_err;

public:
  json_decoder();
  ~json_decoder();
  decoding_status consume_buffer(
      const byte *buf,
      size_t *bytecount) override;
  std::unique_ptr<node> extract_node() override;
  std::unique_ptr<err> extract_error() override;
  void reset() override;
};


class json_encoder : public stream_encoder {
  std::stringstream m_obuf;
  std::unique_ptr<err> m_err;
  size_t m_obufpos = 0;
  size_t m_obufend = 0;

  void _clear_obuf();

public:
  json_encoder();
  encoding_status fill_buffer(
      byte *buf,
      size_t *bytecount) override;
  std::unique_ptr<err> extract_error() override;
  void stream_out(const node &n, std::ostream &os);
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_JSON_HPP_ */
