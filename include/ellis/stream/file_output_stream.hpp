/*
 * @file ellis/stream/file_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/stream/fd_output_stream.hpp>

namespace ellis {


class file_output_stream : public sync_output_stream {
  std::unique_ptr<fd_output_stream> m_fdos;
  int m_fd = -1;

public:
  file_output_stream(const char *filename);

  ~file_output_stream();

  bool next_output_buf(byte **buf, size_t *bytecount) override;

  bool emit(size_t bytecount) override;

  std::unique_ptr<err> extract_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_ */
