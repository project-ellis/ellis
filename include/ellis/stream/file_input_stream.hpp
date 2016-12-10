/*
 * @file ellis/stream/file_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FILE_INPUT_STREAM_HPP_
#define ELLIS_STREAM_FILE_INPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/stream/fd_input_stream.hpp>

namespace ellis {


class file_input_stream : public sync_input_stream {
  std::unique_ptr<fd_input_stream> m_fdis;
public:
  file_input_stream(const char *);

  ~file_input_stream();

  bool next_input_buf(byte **buf, size_t *bytecount) override;

  void put_back(size_t bytecount) override;

  std::unique_ptr<err> extract_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FILE_INPUT_STREAM_HPP_ */
