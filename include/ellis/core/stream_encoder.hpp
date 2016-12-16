/*
 * @file ellis/core/stream_encoder.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_STREAM_ENCODER_HPP_
#define ELLIS_CORE_STREAM_ENCODER_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


enum class encoding_status {
  CONTINUE,
  END,
  ERROR
};

std::ostream & operator<<(std::ostream & os, const encoding_status &s);
std::ostream & operator<<(std::ostream & os, const array_node & v);


// aka serialize, aka output
class stream_encoder {
public:
  /** This function is called by the data stream to tell the encoder it may
   * encode up to bytecount bytes of data into the provided buffer.
   *
   * The callee should use up to the given size, as needed, for encoding, and
   * update bytecount to reflect the amount of unused buffer space left over.
   *
   * If there has been a non-recoverable error in the encoding process, the
   * ERROR status will be returned; otherwise, if an ellis node has been
   * completely encoded, a status of END is returned; otherwise a status of
   * CONTINUE will be returned (to indicate that additional space is needed
   * for the encoding--to be provided via additional calls to fill_buffer).
   *
   * If a status of ERROR is returned, the extract_error() function may be
   * used to access the details of the error.
   *
   * If a status of END or ERROR is returned, then the decoder must be reset
   * before any further calls to fill_buffer().
   */
  virtual encoding_status fill_buffer(
      byte *buf,
      size_t *bytecount) = 0;

  /** Return the error details.  Caller owns it now.
   */
  virtual std::unique_ptr<err> extract_error() = 0;

  /** Reset the encoder to start encoding new_node into output buffers. */
  virtual void reset(const node *new_node) = 0;

  virtual ~stream_encoder() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_STREAM_ENCODER_HPP_ */
