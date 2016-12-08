/*
 * @file stream/cpp_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_CPP_INPUT_STREAM_HPP_
#define ELLIS_STREAM_CPP_INPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/sync_input_stream.hpp>
#include <istream>
#include <memory>

namespace ellis {


class cpp_input_stream : public sync_input_stream {
  std::istream &m_is;
  byte m_buf[4096];
  int m_avail = 0;
  int m_pos = 0;
  std::unique_ptr<err> m_err;
public:
  cpp_input_stream(std::istream &is);

  bool next_input_buf(byte **buf, int *bytecount) override;

  void put_back(int bytecount) override;

  std::unique_ptr<err> get_input_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_CPP_INPUT_STREAM_HPP_ */