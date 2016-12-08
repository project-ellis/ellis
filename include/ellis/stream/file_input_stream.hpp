/*
 * @file stream/file_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FILE_INPUT_STREAM_HPP_
#define ELLIS_STREAM_FILE_INPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <memory>

namespace ellis {


class file_input_stream : public sync_input_stream {
  std::unique_ptr<fd_input_stream> m_fdis;
public:
  file_input_stream(const char *);

  ~file_input_stream();

  bool next_input_buf(byte **buf, int *bytecount) override;

  void put_back(int bytecount) override;

  std::unique_ptr<err> get_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FILE_INPUT_STREAM_HPP_ */
