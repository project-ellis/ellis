#include <ellis/codec/elm327.hpp>

#include <ctype.h>
#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/defs.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/system.hpp>
#include <ellis/private/codec/obd/pid.hpp>
#include <ellis/private/core/err.hpp>
#include <ellis/private/using.hpp>

namespace ellis {

using namespace obd;


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
    const string &msg = ELLIS_SSTRING("value " << val << "is not a valid hex char");
    MAKE_ELLIS_ERR(err_code::VALUE_NOT_HEX, msg);
  }
  ELLIS_ASSERT_UNREACHABLE();
}


elm327_decoder::elm327_decoder() :
  m_node(new node(type::ARRAY))
{
}


std::unique_ptr<node> elm327_decoder::make_obd_node(
    const byte *start, size_t bytecount)
{
  /* TODO: how to handle timestamps? */

  /* Make sure the input is formatted as:
   * AB CD EF ...
   * Where AB specifies the mode, CD specifies the pid, and EF ... are
   * between 1 and 4 space-separated hex pairs.
   */
  if (bytecount < 8) {
    return nullptr;
  }
  const byte *end = start + bytecount;
  for (const byte *p = start; p <= end - 2; p += 3) {
    if (! ( isxdigit(*p) && isxdigit(*(p+1)) ) ) {
      return nullptr;
    }
    if ((p < end-2) && (*(p+2) != ' ')) {
      return nullptr;
    }
  }

  const byte *p = start;
  uint16_t mode = 16*hex_val(*p) + hex_val(*(p+1));
  unique_ptr<string> mode_str = get_mode_string(mode);
  if (mode_str == nullptr) {
    return nullptr;
  }
  p += 3;

  uint16_t pid = 16*hex_val(*p) + hex_val(*(p+1));
  const char *pid_str = get_pid_string(pid);
  if (pid_str == nullptr) {
    return nullptr;
  }
  p += 3;

  uint32_t val = 0;
  size_t bits = 0;
  for (; p < end; p += 3) {
    val = val*16 + hex_val(*p);
    val = val*16 + hex_val(*(p+1));
    bits += 8;
  }
  /* decode_value expects values to be in the MSB, so for a 2-byte value, it
   * should look like:
   * BBBBXXXX
   * where BB are the value bytes that you care about.
   * However, the ELM327 would instead give bytes like this:
   * XXXXBBBB
   * So, compute the number of bytes we have and left-shift.
   */
  val <<= 32 - bits;
  double dec_val = decode_value(pid, val);

  unique_ptr<node> m = unique_ptr<node>(new node(type::MAP));
  m->as_mutable_map().insert("mode", std::move(*mode_str));
  m->as_mutable_map().insert("pid", pid_str);
  m->as_mutable_map().insert("value", dec_val);

  return m;
}


decoding_status elm327_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  if (bytecount == nullptr || *bytecount == 0) {
    return decoding_status::ERROR;
  }

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

    unique_ptr<node> node = make_obd_node(start, p - start);
    if (node == nullptr) {
      m_err.reset(
        new MAKE_ELLIS_ERR(err_code::PARSING_ERROR, "Failed to parse OBD II node"));
      return decoding_status::ERROR;
    }
    m_node->as_mutable_array().append(*node);

    start = p + 1;
  }

  if (last_newline < end-1) {
    *bytecount = end - last_newline;
    return decoding_status::CONTINUE;
  }

  *bytecount = 0;
  return decoding_status::END;
}


std::unique_ptr<node> elm327_decoder::extract_node()
{
  return std::move(m_node);
}


std::unique_ptr<err> elm327_decoder::extract_error()
{
  return std::move(m_err);
}


void elm327_decoder::reset()
{
  m_node->as_mutable_map().clear();
}


}  /* namespace ellis */
