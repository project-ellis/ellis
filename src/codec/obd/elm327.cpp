#include <ellis/codec/obd/elm327.hpp>

#include <ctype.h>
#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/codec/obd/pid.hpp>
#include <ellis/core/err.hpp>
#include <ellis_private/using.hpp>

namespace ellis {
namespace obd {


/** Returns an integer value given a hex character (lowercase or uppercase). If
 * the given value is not a valid hex character, raise an exception.
 */
byte hex_val(byte val)
{
  if (val >= '0' && val <= '9') {
    return val - '0';
  }
  else if (val >= 'a' && val <= 'f') {
    return 0xA + val - 'a';
  }
  else if (val >= 'A' && val <= 'F') {
    return 0xA + val - 'A';
  }
  else {
    const string &msg = ELLIS_SSTRING("value " << val << " is not a valid hex char");
    THROW_ELLIS_ERR(PARSE_FAIL, msg);
  }
}


elm327_decoder::elm327_decoder() :
  m_node(make_unique<node>(type::ARRAY))
{
}


node elm327_decoder::make_obd_node(const byte *start, size_t bytecount)
{
  /* TODO: how to handle timestamps? */

  /* Make sure the input is formatted as:
   * MM PP AB ...
   * Where MM specifies the mode, PP specifies the pid, and AB ... are
   * between 1 and 4 space-separated hex pairs.
   */
  static constexpr size_t smallest_node = sizeof("MM PP")-1;
  static constexpr size_t largest_node = sizeof("MM PP AA BB CC DD")-1;
  if (bytecount < smallest_node) {
    THROW_ELLIS_ERR(PARSE_FAIL,
        "OBD II node length " << bytecount << " is too small");
  }
  else if (bytecount > largest_node) {
    THROW_ELLIS_ERR(PARSE_FAIL,
        "OBD II node length " << bytecount << " is too large");
  }
  const byte *end = start + bytecount;
  for (const byte *p = start; p <= end-2; p += 3) {
    if (! ( isxdigit(*p) && isxdigit(*(p+1)) ) ) {
      THROW_ELLIS_ERR(PARSE_FAIL,
          "OBD II node has non-hex digits");
    }
    if ((p < end-2) && (*(p+2) != ' ')) {
      THROW_ELLIS_ERR(PARSE_FAIL,
          "OBD II node is not space-separated");
    }
  }

  const byte *p = start;
  uint16_t mode = 16*hex_val(*p) + hex_val(*(p+1));
  const string &mode_str = get_mode_string(mode);
  p += 3;

  uint16_t pid = 16*hex_val(*p) + hex_val(*(p+1));
  const char *pid_str = get_pid_string(pid);
  p += 3;

  node n = node(type::MAP);
  map_node &m = n.as_mutable_map();
  m.insert("mode", mode_str);
  m.insert("pid", pid_str);
  if (p < end) {
    uint32_t val = 0;
    size_t shift = 32;
    for (; p < end; p += 3) {
      val = val*16 + hex_val(*p);
      val = val*16 + hex_val(*(p+1));
      shift -= 8;
    }
    /* decode_value expects values to be in the MSB, so for a 2-byte value XXXX,
     * the output value of 4 bytes should look like:
     * XXXX0000
     * However, the ELM327 would instead give bytes like this:
     * 0000XXXX
     * So, compute the number of bits we have and left-shift.
     */
    ELLIS_ASSERT_LT(shift, 32);
    val <<= shift;
    double dec_val = decode_value(pid, val);

    m.insert("value", dec_val);
  }

  return n;
}


node_progress elm327_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  /* We assume that if there are multiple responses coming from the ELM327, then
   * they are separated by newlines.
   */
  const byte *start = buf;
  const byte *end = buf + *bytecount;
  const byte *last_newline = end;
  while (true) {
    const byte *p;
    for (p = start; p < end; p++) {
      if (*p == '\n') {
        last_newline = p;
        break;
      }
    }
    if (p == end) {
      break;
    }

    try {
      node n = make_obd_node(start, p - start);
      m_node->as_mutable_array().append(n);
    }
    catch (const err &e) {
      return node_progress(make_unique<err>(e));
    }

    start = p + 1;
  }

  if (last_newline < end-1) {
    *bytecount = end - last_newline;
    return node_progress(stream_state::CONTINUE);
  }

  *bytecount = 0;
  return node_progress(std::move(m_node));
}


node_progress elm327_decoder::chop()
{
  return node_progress(std::move(m_node));
}


void elm327_decoder::reset()
{
  m_node = make_unique<node>(type::ARRAY);
}


}  /* namespace obd */
}  /* namespace ellis */
