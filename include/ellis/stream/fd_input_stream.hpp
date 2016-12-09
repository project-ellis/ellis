/*
 * @file stream/fd_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FD_INPUT_STREAM_HPP_
#define ELLIS_STREAM_FD_INPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/sync_input_stream.hpp>

namespace ellis {


class fd_input_stream : public sync_input_stream {
  char m_buf[4096];
  int m_fd;
  int m_pos = 0;
  int m_valid = 0;
public:
  fd_input_stream(int fd);
  bool next_input_buf(byte **buf, size_t *bytecount) override;
  void put_back(size_t bytecount) override;
  std::unique_ptr<err> extract_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FD_INPUT_STREAM_HPP_ */
