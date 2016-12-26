/*
 * @file ellis/stream/mem_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_
#define ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/sync_output_stream.hpp>

namespace ellis {


class mem_output_stream : public sync_output_stream {
  byte *m_buf;
  size_t m_len;
  size_t m_pos = 0;
  std::unique_ptr<err> m_err;

public:
  /**
   * Create a memory output stream that will fill in buf, up to len bytes,
   * always making sure to terminate the string after any given call to
   * emit().  This means that the actual maximum length of the string will
   * be one less than len, to allow for a null terminator.
   */
  mem_output_stream(void *buf, size_t len);

  bool next_output_buf(byte **buf, size_t *bytecount) override;
  bool emit(size_t bytecount) override;
  std::unique_ptr<err> extract_output_error() override;
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_MEM_OUTPUT_STREAM_HPP_ */
