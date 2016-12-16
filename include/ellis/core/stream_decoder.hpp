/*
 * @file ellis/core/stream_decoder.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_STREAM_DECODER_HPP_
#define ELLIS_CORE_STREAM_DECODER_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <memory>

namespace ellis {


// TODO: need one specific to stream_decoder that says MAY_CONTINUE and
// MUST_CONTINUE, indicating that it the object may be terminated here or
// may not be terminated here.  Add greedy param to consume_buffer, and
// return remaining chars, for reinvocation...  Allows us to do a more
// streaming/generator approach, and/or decode headers of huge objects.
// Is there any need for analogous greedy param to encoders?
enum class decoding_status {
  MAY_CONTINUE,
  MUST_CONTINUE,
  END,
  ERROR
};

std::ostream & operator<<(std::ostream & os, const array_node & v);


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
   * MUST_CONTINUE will be returned (to indicate that additional bytes must be
   * provided via additional calls to consume_buffer).
   *
   * If a status of END is returned, the constructed object may be obtained
   * via the extract_node() function.
   *
   * If a status of ERROR is returned, the extract_error() function may be
   * used to access the details of the error.
   *
   * If a status of END or ERROR is returned, then the decoder must be reset
   * before any further calls to consume_buffer().
   */
  virtual decoding_status consume_buffer(
      const byte *buf,
      size_t *bytecount) = 0;

  /** Check on the node being constructed.
   *
   * Whether there is any useful partially constructed node to see before
   * consume_buffer has returned COMPLETE is dependent on the particular
   * codec.  If there is no useful partial node to see, the code may
   * return nullptr to indicate that there is nothing to see, which is
   * the default behavior.
   */
  virtual const node * peek_node() { return nullptr; }

  /** Return the constructed node.
   *
   * Only valid when decoder status is END or MAY_CONTINUE.
   */
  virtual std::unique_ptr<node> extract_node() = 0;

  /** Return the error details.  Caller owns it now.
   *
   * Only valid when decoder status is ERROR.
   */
  virtual std::unique_ptr<err> extract_error() = 0;

  /** Reset the encoder to start encoding a new ellis node.
   *
   * This should result in the same behavior as constructing a new decoder,
   * but may be more efficient.
   */
  virtual void reset() = 0;

  virtual ~stream_decoder() {}
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_STREAM_DECODER_HPP_ */
