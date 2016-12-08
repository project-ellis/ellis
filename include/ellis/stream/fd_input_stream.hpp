/*
 * @file stream/fd_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_FD_INPUT_STREAM_HPP_
#define ELLIS_STREAM_FD_INPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/sync_input_stream.hpp>

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
  std::unique_ptr<err> get_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_FD_INPUT_STREAM_HPP_ */
