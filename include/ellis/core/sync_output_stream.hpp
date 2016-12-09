/*
 * @file sync_output_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_SYNC_OUTPUT_STREAM_HPP_
#define ELLIS_SYNC_OUTPUT_STREAM_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


/** Abstract API for synchronous (blocking) output streams.
 *
 * Note--a class may implement both sync_input_stream and sync_output_stream.
 */
class sync_output_stream {
public:
  /** Get another buffer, suitable for output to the stream, filling in buf
   * and bytecount for the block of data.  Buffer remains owned by the stream;
   * caller should not call next_output_buf() again until finished with the
   * buffer.
   *
   * Return values:
   *   true  --> Buffer is available (and has been returned).
   *   false --> Buffer unavailable (check extract_output_error() for details).
   */
  virtual bool next_output_buf(byte **buf, size_t *bytecount) = 0;

  /** Send bytecount bytes of data stored in the buffer (the last buffer
   * returned by next_output_buf).
   *
   * Return values:
   *   true  --> Data successfully written.
   *   false --> Problem writing (check extract_output_error() for details).
   */
  virtual bool emit(size_t bytecount) = 0;

  /** Return the error details.  Caller owns it now.  */
  virtual std::unique_ptr<err> extract_output_error() = 0;

  virtual ~sync_output_stream() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_SYNC_OUTPUT_STREAM_HPP_ */
