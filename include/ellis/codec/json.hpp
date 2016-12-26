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
#include <ellis/core/decoder.hpp>
#include <ellis/core/encoder.hpp>
#include <sstream>

namespace ellis {


class json_parser;
class json_tokenizer;

class json_decoder : public decoder {

  std::unique_ptr<json_tokenizer> m_toker;
  std::unique_ptr<json_parser> m_parser;

public:
  json_decoder();
  ~json_decoder();
  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;
  node_progress chop() override;
  void reset() override;
};


class json_encoder : public encoder {
  std::stringstream m_obuf;
  size_t m_obufpos = 0;
  size_t m_obufend = 0;

  void _clear_obuf();
  void _stream_out(const node &n, std::ostream &os);

public:
  json_encoder();
  progress fill_buffer(
      byte *buf,
      size_t *bytecount) override;
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_JSON_HPP_ */
