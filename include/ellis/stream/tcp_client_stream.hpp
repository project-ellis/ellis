/*
 * @file stream/tcp_client_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_TCP_CLIENT_STREAM_HPP_
#define ELLIS_STREAM_TCP_CLIENT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/stream/fd_input_stream.hpp>
#include <ellis/stream/fd_output_stream.hpp>
#include <memory>

namespace ellis {


class tcp_stream : public sync_input_stream, sync_output_stream {
  std::unique_ptr<fd_input_stream> m_fdis;
  std::unique_ptr<fd_output_stream> m_fdos;
public:
  tcp_stream(const char *, const char *);

  ~tcp_stream();

  bool next_input_buf(byte **buf, int *bytecount) override;

  void put_back(int bytecount) override;

  std::unique_ptr<err> get_input_error() override;

  bool next_output_buf(byte **buf, int *bytecount) override;

  bool emit(int bytecount) override;

  std::unique_ptr<err> get_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_TCP_CLIENT_STREAM_HPP_ */
