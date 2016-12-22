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
    throw MAKE_ELLIS_ERR(err_code::PARSING_ERROR, msg);
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


decoding_status can_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  if (bytecount == nullptr || *bytecount == 0) {
    m_err.reset(
      new MAKE_ELLIS_ERR(err_code::PARSING_ERROR, "Cannot parse empty OBD II node"));
    return decoding_status::ERROR;
  }

  /* ECU responses are 8 bytes long. */
  if (*bytecount < 8) {
    *bytecount = 0;
    return decoding_status::CONTINUE;
  }

  const byte *end = buf + *bytecount;
  for (const byte *p = buf; p < end; p += 8) {
    try {
      node n = make_obd_node(p);
      m_node->as_mutable_array().append(n);
    }
    catch (const err &e) {
      m_err.reset(new err(e));
      return decoding_status::ERROR;
    }
  }

  if ((*bytecount % 8) != 0) {
    *bytecount = *bytecount - (8*(*bytecount/8));
    return decoding_status::CONTINUE;
  }
  else {
    *bytecount = 0;
    return decoding_status::END;
  }
}


decoding_status can_decoder::terminate_stream()
{
  return decoding_status::END;
}


std::unique_ptr<node> can_decoder::extract_node()
{
  return std::move(m_node);
}


std::unique_ptr<err> can_decoder::extract_error()
{
  return std::move(m_err);
}


void can_decoder::reset()
{
  m_node->as_mutable_map().clear();
}


}  /* namespace obd */
}  /* namespace ellis */
