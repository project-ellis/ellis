/*
 * @file stream_decoder.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_STREAM_DECODER_HPP_
#define ELLIS_STREAM_DECODER_HPP_

#include <ellis/defs.hpp>
#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <memory>

namespace ellis {


// TODO: need one specific to stream_decoder that says MAY_CONTINUE and
// MUST_CONTINUE, indicating that it the object may be terminated here or
// may not be terminated here.  Add greedy param to consume_buffer, and
// return remaining chars, for reinvocation...  Allows us to do a more
// streaming/generator approach, and/or decode headers of huge objects.
// Is there any need for analogous greedy param to encoders?
enum class decoding_status {
  CONTINUE,
  END,
  ERROR
};


// aka deserialize, aka input
class stream_decoder {
public:
  /** This function is called by the data stream to tell the decoder it may
   * decode up to bytecount bytes of data from the provided buffer.
   *
   * The callee should attempt to decode the given data to construct an ellis
   * node, and update the size parameter to specify the number of bytes that
   * remain unused and available, if there are any.  Whatever bytes are left
   * will be reclaimed by the stream for use by the next consumer.
   *
   * If there has been a non-recoverable error in the decoding process, the
   * ERROR status will be returned; otherwise, if an ellis node has been
   * completely decoded, a status of END is returned; otherwise a status of
   * CONTINUE will be returned (to indicate that additional bytes must be
   * provided via additional calls to consume_buffer).
   *
   * If a status of END is returned, the constructed object may be obtained
   * via the get_node() function.
   *
   * If a status of ERROR is returned, the get_error() function may be used to
   * access the details of the error.
   *
   * If a status of END or ERROR is returned, then the decoder must be reset
   * before any further calls to consume_buffer().
   */
  virtual decoding_status consume_buffer(
      const byte *buf,
      // TODO: martin likes size_t
      int *bytecount) = 0;

  /** Return the constructed node.
   *
   * It is only valid to call this when consume_buffer has returned COMPLETE.
   */
  virtual std::unique_ptr<node> get_node() = 0;

  /** Return the error details.  Caller owns it now.
   */
  virtual std::unique_ptr<err> get_error() = 0;

  /** Reset the encoder to start encoding a new ellis node.
   *
   * This should result in the same behavior as constructing a new decoder,
   * but may be more efficient.
   */
  virtual void reset() = 0;

  virtual ~stream_decoder() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_STREAM_DECODER_HPP_ */
