#undef NDEBUG
#include <ellis/codec/obd/can.hpp>
#include <ellis/codec/obd/elm327.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis_private/codec/obd/pid.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>

int main() {
  using namespace ellis;
  using namespace ellis::obd;

  auto parse_fail = [](std::function<void()> fn)
  {
    bool threw = false;
    try {
      fn();
    } catch(const err &e) {
      if (e.code() == err_code::PARSING_ERROR) {
        threw = true;
      }
    }
    ELLIS_ASSERT_TRUE(threw);
  };


  /* OBD. */
  parse_fail([]{
    get_mode_string(0x00);
  });
  parse_fail([]{
    get_mode_string(0xFF);
  });

  ELLIS_ASSERT_EQ(get_mode_string(0x41), "current");
  ELLIS_ASSERT_EQ(get_mode_string(0x42), "freeze");

  ELLIS_ASSERT_EQ(string(get_pid_string(0x0D)), "vehicle_speed");

  ELLIS_ASSERT_DBL_EQ(decode_value(0x05, 0xAB000000), 0xAB - 40);
  ELLIS_ASSERT_DBL_EQ(decode_value(0x05, 0xAB123456), 0xAB - 40);
  ELLIS_ASSERT_DBL_EQ(decode_value(0x05, 0x00000000), -40);

  ELLIS_ASSERT_EQ(decode_value(0x0C, 0xABCD0000), (0xAB*256 + 0xCD)/4.0);
  ELLIS_ASSERT_EQ(decode_value(0x0C, 0xABCD1234), (0xAB*256 + 0xCD)/4.0);

  ELLIS_ASSERT_DBL_EQ(decode_value(0x0D, 0xAB000000), 0xAB);
  ELLIS_ASSERT_DBL_EQ(decode_value(0x0D, 0xAB123456), 0xAB);

  ELLIS_ASSERT_EQ(decode_value(0x10, 0xABCD0000), (0xAB*256 + 0xCD)/100.0);
  ELLIS_ASSERT_EQ(decode_value(0x10, 0xABCD1234), (0xAB*256 + 0xCD)/100.0);

  ELLIS_ASSERT_EQ(decode_value(0x14, 0xAB000000), 0xAB / 200.0);
  ELLIS_ASSERT_EQ(decode_value(0x14, 0xAB123456), 0xAB / 200.0);

  /* CAN. */
  {
    can_decoder dec;
    const byte buf[] = {0};
    size_t count = 0;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::ERROR);
    ELLIS_ASSERT_NOT_NULL(dec.extract_error().get());
  }

  {
    can_decoder dec;
    const byte buf[] = { 0x3, 0x41, 0x05, 0xB9, 0xA, 0xB, 0xC, 0x0 };
    size_t count = sizeof(buf);
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_EQ(count, 0);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "current");
    ELLIS_ASSERT_EQ(m["pid"], "engine_coolant_temp");
    ELLIS_ASSERT_DBL_EQ(m["value"], 0xB9 - 40);
  }

  {
    can_decoder dec;
    const byte buf[] = { 0x3, 0x42, 0x05, 0xB9, 0xA, 0xB, 0xC, 0x55 };
    size_t count = sizeof(buf);
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_EQ(count, 0);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "freeze");
    ELLIS_ASSERT_EQ(m["pid"], "engine_coolant_temp");
    ELLIS_ASSERT_DBL_EQ(m["value"], 0xB9 - 40);
  }

  {
    can_decoder dec;
    const byte buf[] = { 0x4, 0x41, 0x0C, 0x08, 0x1B, 0xA, 0xB, 0x0 };
    size_t count = sizeof(buf);
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_EQ(count, 0);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "current");
    ELLIS_ASSERT_EQ(m["pid"], "engine_rpm");
    ELLIS_ASSERT_DBL_EQ(m["value"], (0x08*256 + 0x1B)/4.0);
  }

  /* ELM327. */
  {
    elm327_decoder dec;
    const byte buf[] = "41 05 B9\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_EQ(count, 0);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "current");
    ELLIS_ASSERT_EQ(m["pid"], "engine_coolant_temp");
    ELLIS_ASSERT_DBL_EQ(m["value"], 0xB9 - 40);
  }

  {
    elm327_decoder dec;
    const byte buf[] = "41 0C 08 1B\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_EQ(count, 0);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "current");
    ELLIS_ASSERT_EQ(m["pid"], "engine_rpm");
    ELLIS_ASSERT_DBL_EQ(m["value"], (0x08*256 + 0x1B)/4.0);
  }

  {
    elm327_decoder dec;
    const byte buf[] = "";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::ERROR);
    ELLIS_ASSERT_NOT_NULL(dec.extract_error().get());
  }

  {
    elm327_decoder dec;
    const byte buf[] = "\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::ERROR);
    ELLIS_ASSERT_NOT_NULL(dec.extract_error().get());
  }

  {
    elm327_decoder dec;
    const byte buf[] = "41\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::ERROR);
    ELLIS_ASSERT_NOT_NULL(dec.extract_error().get());
  }

  {
    elm327_decoder dec;
    const byte buf[] = "41 0C\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::END);
    ELLIS_ASSERT_NULL(dec.extract_error().get());
    node n = *dec.extract_node();
    ELLIS_ASSERT_EQ(n.as_array().length(), 1);
    const map_node &m = n.as_array()[0].as_map();
    ELLIS_ASSERT_EQ(m["mode"], "current");
    ELLIS_ASSERT_EQ(m["pid"], "engine_rpm");
    ELLIS_ASSERT_EQ(m.has_key("value"), false);
  }

  {
    elm327_decoder dec;
    const byte buf[] = "41 0C 12 34 56 78 9A\n";
    size_t count = sizeof(buf) - 1;
    decoding_status status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status, decoding_status::ERROR);
    ELLIS_ASSERT_NOT_NULL(dec.extract_error().get());
  }

  return 0;
}
