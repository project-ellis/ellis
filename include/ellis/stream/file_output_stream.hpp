/*
 * @file stream/file_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <memory>

namespace ellis {


class file_output_stream : public sync_output_stream {
  std::unique_ptr<fd_output_stream> m_fdos;
public:
  file_output_stream(const char *);

  ~file_output_stream();

  bool next_output_buf(byte **buf, int *bytecount) override;

  bool emit(int bytecount) override;

  std::unique_ptr<err> get_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FILE_OUTPUT_STREAM_HPP_ */
