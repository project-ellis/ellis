/*
 * @file sync_input_stream.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_SYNC_INPUT_STREAM_HPP_
#define ELLIS_SYNC_INPUT_STREAM_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <memory>

namespace ellis {


/** Abstract API for synchronous (blocking) input streams.
 *
 * Note--a class may implement both sync_input_stream and sync_output_stream.
 */
class sync_input_stream {
public:
  /** Get another block of data from the stream, filling in buf and bytecount
   * for the block of data.  Buffer remains owned by the stream; caller
   * should not call next_input_buf() again until finished with the buffer.
   *
   * Return values:
   *   true  --> Data is available (and has been returned).
   *   false --> Data is unavailable (check get_input_error() for details).
   */
  virtual bool next_input_buf(byte **buf, int *bytecount) = 0;

  /** Put back data at the end of the block last returned from next_block(). */
  virtual void put_back(int bytecount) = 0;

  /** Return the error details.  Caller owns it now.  */
  virtual std::unique_ptr<err> get_input_error() = 0;

  virtual ~sync_input_stream() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_SYNC_INPUT_STREAM_HPP_ */
