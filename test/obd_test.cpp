#undef NDEBUG
#include <assert.h>
#include <ellis/codec/elm327.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/private/codec/obd/pid.hpp>
#include <ellis/private/using.hpp>
#include <test/assert.hpp>

int main() {
  using namespace ellis;
  using namespace ellis::obd;
  using namespace ellis::test;

  /* OBD. */
  assert(*get_mode_string(0x7F) == "unknown");
  assert(get_mode_string(0x00) == nullptr);
  assert(*get_mode_string(0x41) == "current");
  assert(*get_mode_string(0x42) == "freeze");
  assert(*get_mode_string(0xFF) == std::to_string(0xFF - 0x40));

  assert(get_pid_string(0xFF) == nullptr);
  assert(string(get_pid_string(0x0D)) == "vehicle_speed");

  assert(dbl_equal(decode_value(0x05, 0xAB000000), 0xAB - 40));
  assert(dbl_equal(decode_value(0x05, 0xAB123456), 0xAB - 40));
  assert(dbl_equal(decode_value(0x05, 0x00000000), -40));

  assert(decode_value(0x0C, 0xABCD0000) == (0xAB*256 + 0xCD)/4.0);
  assert(decode_value(0x0C, 0xABCD1234) == (0xAB*256 + 0xCD)/4.0);

  assert(dbl_equal(decode_value(0x0D, 0xAB000000), 0xAB));
  assert(dbl_equal(decode_value(0x0D, 0xAB123456), 0xAB));

  assert(decode_value(0x10, 0xABCD0000) == (0xAB*256 + 0xCD)/100.0);
  assert(decode_value(0x10, 0xABCD1234) == (0xAB*256 + 0xCD)/100.0);

  assert(decode_value(0x14, 0xAB000000) == 0xAB / 200.0);
  assert(decode_value(0x14, 0xAB123456) == 0xAB / 200.0);

  /* ELM327. */
  {
    elm327_decoder dec;
    const byte buf[] = "41 05 B9\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    assert(status == decoding_status::END);
    assert(count == 0);
    assert(dec.extract_error() == nullptr);
    node n = *dec.extract_node();
    assert(n.as_array().length() == 1);
    const map_node &m = n.as_array()[0].as_map();
    assert(m["mode"] == "current");
    assert(m["pid"] == "engine_coolant_temp");
    assert(dbl_equal(m["value"], 0xB9 - 40));
  }

  {
    elm327_decoder dec;
    const byte buf[] = "41 0C 08 1B\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    assert(status == decoding_status::END);
    assert(count == 0);
    assert(dec.extract_error() == nullptr);
    node n = *dec.extract_node();
    assert(n.as_array().length() == 1);
    const map_node &m = n.as_array()[0].as_map();
    assert(m["mode"] == "current");
    assert(m["pid"] == "engine_rpm");
    assert(dbl_equal(m["value"], (0x08*256 + 0x1B)/4.0));
  }

  {
    elm327_decoder dec;
    const byte buf[] = "";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    assert(status == decoding_status::ERROR);
    assert(dec.extract_error() != nullptr);
  }

  {
    elm327_decoder dec;
    const byte buf[] = "\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    assert(status == decoding_status::ERROR);
    assert(dec.extract_error() != nullptr);
  }

  return 0;
}
