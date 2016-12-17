/*
 * @file codec/can.hpp
 *
 * @brief Ellis CAN codec C++ header.
 *
 * This decoder implements decoding of ECU responses in the OBD II protocol over
 * CAN.  Specifically, it currently implements only OBD II modes 01 and 02 (SAE
 * Standard modes) and does not implement anything vehicle-specific.
 */

#pragma once
#ifndef ELLIS_CODEC_CAN_HPP_
#define ELLIS_CODEC_CAN_HPP_

#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/stream_decoder.hpp>
#include <ellis/core/stream_encoder.hpp>

namespace ellis {
namespace obd {


class can_decoder : public stream_decoder {
  std::unique_ptr<node> m_node;
  std::unique_ptr<err> m_err;

  /** Makes an OBD II node from a given byte sequence.
   *
   * @param start a pointer to the start of the byte sequence
   *
   * @return a new node representing an OBD II datum
   * */
  static node make_obd_node(const void *start);

public:
  /** Constructor. */
  can_decoder();

  /** Consumes a given buffer of data coming from the CAN bus. Each buffer
   * represents an ECU response to a query.
   *
   * For more details regarding the OBD II, see the standard or:
   *
   * https://en.wikipedia.org/wiki/OBD-II_PIDs
   *
   * In particular:
   * https://en.wikipedia.org/wiki/OBD-II_PIDs#Response
   */
  decoding_status consume_buffer(
      const byte *buf,
      size_t *bytecount) override;

  std::unique_ptr<node> extract_node() override;

  std::unique_ptr<err> extract_error() override;

  void reset() override;
};


}  /* namespace obd */
}  /* namespace ellis */

#endif  /* ELLIS_CODEC_CAN_HPP_ */
