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



struct ecu_response {
  byte extra_bytes;
  byte mode;
  byte pid;
  byte data[4];
  byte unused;
};


can_decoder::can_decoder() :
  m_node(new node(type::ARRAY))
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
    const string &msg = ELLIS_SSTRING(
        "Non-SAE standard extra bytes field: " << resp->extra_bytes);
    throw MAKE_ELLIS_ERR(PARSING_ERROR, msg);
  }
  double dec_val = decode_value(resp->pid, data);

  if (resp->unused != 0x00 && resp->unused != 0x55) {
    ELLIS_LOG(WARN, "PID %s has non-compliant unused field %hh", pid_str, resp->unused);
  }

  node m = node(type::MAP);
  m.as_mutable_map().insert("mode", mode_str);
  m.as_mutable_map().insert("pid", pid_str);
  m.as_mutable_map().insert("value", dec_val);

  return m;
}


node_progress can_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  if (bytecount == nullptr || *bytecount == 0) {
  return node_progress(make_unique<err>(MAKE_ELLIS_ERR(
        PARSING_ERROR, "Cannot parse empty OBD II node")));
  }

  /* ECU responses are 8 bytes long. */
  if (*bytecount < 8) {
    *bytecount = 0;
    return node_progress(stream_state::CONTINUE);
  }

  const byte *end = buf + *bytecount;
  for (const byte *p = buf; p < end; p += 8) {
    try {
      node n = make_obd_node(p);
      m_node->as_mutable_array().append(n);
    }
    catch (const err &e) {
      return node_progress(make_unique<err>(e));
    }
  }

  if ((*bytecount % 8) != 0) {
    *bytecount = *bytecount - (8*(*bytecount/8));
    return node_progress(stream_state::CONTINUE);
  }
  else {
    *bytecount = 0;
    return node_progress(std::move(m_node));
  }
}


node_progress can_decoder::cleave()
{
  return node_progress(make_unique<err>(MAKE_ELLIS_ERR(
        PARSING_ERROR, "TODO: martin verify should fail")));
}


void can_decoder::reset()
{
  m_node.reset(new node(type::ARRAY));
}


}  /* namespace obd */
}  /* namespace ellis */
