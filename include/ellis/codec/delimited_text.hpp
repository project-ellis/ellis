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
#include <ellis/core/stream_decoder.hpp>
#include <ellis/core/stream_encoder.hpp>
#include <sstream>

namespace ellis {


class delimited_text_decoder : public stream_decoder {
  std::unique_ptr<node> m_node;
  std::ostringstream m_ss;
  std::unique_ptr<err> m_err;

  void _clear_ss();

public:
  // TODO: vector of delimiter strings (first level, second, ...)
  // e.g. delimited_text_decoder({"\n", " ", ","}) produces an array of arrays
  // of arrays.
  delimited_text_decoder();

  node_progress consume_buffer(
      const byte *buf,
      size_t *bytecount) override;

  node_progress cleave() override;

  void reset() override;
};


class delimited_text_encoder : public stream_encoder {
  std::stringstream m_ss;
  std::unique_ptr<err> m_err;
  size_t m_sspos = 0;
  size_t m_ssend = 0;

  void _clear_ss();

public:
  delimited_text_encoder();

  ~delimited_text_encoder();

  encoding_status fill_buffer(
      byte *buf,
      size_t *bytecount) override;

  std::unique_ptr<err> extract_error() override;

  void reset(const node *new_node) override;
};


}  /* namespace ellis */

#endif  /* ELLIS_CODEC_DELIMITED_TEXT_HPP_ */
