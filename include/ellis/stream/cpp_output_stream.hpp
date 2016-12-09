/*
 * @file stream/cpp_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_CPP_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_CPP_OUTPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/sync_output_stream.hpp>

namespace ellis {


class cpp_output_stream : public sync_output_stream {
  std::ostream &m_os;
  byte m_buf[4096];
  std::unique_ptr<err> m_err;
public:
  cpp_output_stream(std::ostream &os);

  bool next_output_buf(byte **buf, size_t *bytecount) override;

  bool emit(size_t bytecount) override;

  std::unique_ptr<err> extract_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_CPP_OUTPUT_STREAM_HPP_ */
