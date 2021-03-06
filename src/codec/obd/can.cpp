/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <ellis/codec/obd/can.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/codec/obd/pid.hpp>
#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

#include <cstring>

namespace ellis {
namespace obd {


/** The response from a vehicle ECU. */
struct ecu_response {
  /** Data length. */
  byte extra_bytes;
  /** A CAN PID mode. */
  byte mode;
  /** A CAN PID. */
  byte pid;
  /* PID data. */
  byte data[4];
  /* Unused. */
  byte unused;
};


can_decoder::can_decoder() :
  m_node(make_unique<node>(type::ARRAY))
{
}


node can_decoder::make_obd_node(const void *start)
{
  const ecu_response *resp = static_cast<const ecu_response *>(start);
  const string &mode_str = get_mode_string(resp->mode);
  const char *pid_str = get_pid_string(resp->pid);

  uint32_t data;
  if (resp->extra_bytes == 3) {
    data = resp->data[0] << 24;
  }
  else if (resp->extra_bytes == 4) {
    data =
      (resp->data[0] << 24) |
      (resp->data[1] << 16);
  }
  else if (resp->extra_bytes == 5) {
    data =
      (resp->data[0] << 24) |
      (resp->data[1] << 16) |
      (resp->data[2] << 8);
  }
  else if (resp->extra_bytes == 6) {
    data =
      (resp->data[0] << 24) |
      (resp->data[1] << 16) |
      (resp->data[2] << 8) |
      (resp->data[3] << 0);
  }
  else {
    /* In SAE Standard, we must have 3 <= extra bytes <= 6. */
    THROW_ELLIS_ERR(PARSE_FAIL,
        "Non-SAE standard extra bytes value: " << resp->extra_bytes);
  }
  double dec_val = decode_value(resp->pid, data);

  if (resp->unused != 0x00 && resp->unused != 0x55) {
    ELLIS_LOG(WARN, "PID %s has non-compliant unused field %hh", pid_str, resp->unused);
  }

  node n = node(type::MAP);
  map_node &m = n.as_mutable_map();
  m.insert("mode", mode_str);
  m.insert("pid", pid_str);
  m.insert("value", dec_val);

  return n;
}


node_progress can_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  /* ECU responses are 8 bytes long. */
  if (*bytecount < 8) {
    return node_progress(stream_state::CONTINUE);
  }

  const byte *end = buf + *bytecount;
  for (const byte *p = buf; p < end; p += 8) {
    try {
      node n = make_obd_node(p);
      m_node->as_mutable_array().append(std::move(n));
    }
    catch (const err &e) {
      return node_progress(make_unique<err>(e));
    }
  }

  if ((*bytecount % 8) != 0) {
    *bytecount -= (8*(*bytecount/8));
    return node_progress(stream_state::CONTINUE);
  }
  else {
    *bytecount = 0;
    return node_progress(std::move(m_node));
  }
}


node_progress can_decoder::chop()
{
  return node_progress(std::move(m_node));
}


void can_decoder::reset()
{
  m_node = make_unique<node>(type::ARRAY);
}


}  /* namespace obd */
}  /* namespace ellis */
