/*
 * @file ellis/stream/cpp_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_CPP_INPUT_STREAM_HPP_
#define ELLIS_STREAM_CPP_INPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/sync_input_stream.hpp>

namespace ellis {


class cpp_input_stream : public sync_input_stream {
  std::istream &m_is;
  byte m_buf[4096];
  int m_avail = 0;
  int m_pos = 0;
  std::unique_ptr<err> m_err;

public:
  cpp_input_stream(std::istream &is);

  bool next_input_buf(const byte **buf, size_t *bytecount) override;

  void put_back(size_t bytecount) override;

  std::unique_ptr<err> extract_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_CPP_INPUT_STREAM_HPP_ */
