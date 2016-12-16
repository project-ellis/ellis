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


class json_decoder : public stream_decoder {

  enum class doc_state {
    INIT,
    MAP_GOT_OPEN,
    MAP_GOT_KEY,
    MAP_GOT_COLON,
    MAP_GOT_VAL,
    MAP_GOT_COMMA,
    ARRAY_GOT_OPEN,
    ARRAY_GOT_VAL,
    ARRAY_GOT_COMMA,
    GOT_VAL,
  };

  enum class token_type {
    LEFT_CURLY,
    RIGHT_CURLY,
    COLON,
    COMMA,
    LEFT_SQUARE,
    RIGHT_SQUARE,
    STRING,
    INTEGER,
    REAL,
    BAREWORD,
  };

  enum class token_state {
    INIT,
    STRING,
    QUOTE,
    QUOTE_U1,
    QUOTE_U2,
    QUOTE_U3,
    QUOTE_U4,
    INT,
    FRAC,
    EXP,
    COMMENTSLASH2,
    COMMENT,
    END,
  };

  int m_sign;
  int m_digitcount;
  int64_t m_int;
  decoding_status m_status;
  std::vector<std::unique_ptr<node>> m_stack;
  token_state m_tokstate;
  token_type m_toktype;
  std::ostringstream m_txt;
  std::unique_ptr<err> m_err;

  void _clear_txt();

public:
  json_decoder();
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
  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_JSON_HPP_ */
