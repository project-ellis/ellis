/*
 * @file ellis/stream/mem_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_MEM_INPUT_STREAM_HPP_
#define ELLIS_STREAM_MEM_INPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
// TODO: none of the streams should need node.hpp
#include <ellis/core/node.hpp>
#include <ellis/core/sync_input_stream.hpp>

namespace ellis {


class mem_input_stream : public sync_input_stream {
  const byte *m_buf;
  size_t m_len;
  size_t m_pos = 0;
  std::unique_ptr<err> m_err;

public:
  mem_input_stream(const void *buf, size_t len);

  bool next_input_buf(const byte **buf, size_t *bytecount) override;
  void put_back(size_t bytecount) override;
  std::unique_ptr<err> extract_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_MEM_INPUT_STREAM_HPP_ */
