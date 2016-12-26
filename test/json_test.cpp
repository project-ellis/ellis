#undef NDEBUG
#include <ellis/codec/json.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <ellis/stream/mem_input_stream.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <sstream>


int main() {
  using namespace ellis;

  // set_system_log_prefilter(log_severity::DBUG);
  json_decoder dec;
  json_encoder enc;

  auto ser_deser = [&dec, &enc](const string & s) {
    ELLIS_LOG(NOTI, "===========================================");
    auto n = load_mem(s.c_str(), s.size(), dec);
    size_t len2 = s.size() * 2 + 1;
    unique_ptr<char[]> buf2 (new char[len2]);
    dump_mem(n.get(), buf2.get(), len2, enc);
    auto s2 = std::string(buf2.get());
    ELLIS_ASSERT_EQ(s, s2);
    std::stringstream ss3;
    dump(n.get(), cpp_output_stream(ss3), enc);
    auto s3 = ss3.str();
    ELLIS_ASSERT_EQ(s, s3);
  };

  ser_deser("null");
  ser_deser("true");
  ser_deser("false");
  ser_deser("-9223372036854775808");
  ser_deser("-1");
  ser_deser("0");
  ser_deser("1");
  ser_deser("9223372036854775807");
  ser_deser("0.000000");
  // TODO: enable XeY tests after overhaul double output
  // ser_deser("2.5e15");
  ser_deser("2.500000");
  // ser_deser("2.5e-15");
  // ser_deser("-0.25e1");
  ser_deser("-2.500000");
  // ser_deser("-0.25e-1");
  ser_deser(R"("")");
  ser_deser(R"("simple")");
  ser_deser(R"("s\"im\"ple")");
  ser_deser(R"("sim\nple")");
  ser_deser(R"("sim★ple")");
  ser_deser(R"({})");
  ser_deser(R"({ "map": {} })");
  ser_deser(R"({ "map": [] })");
  ser_deser(R"({ "map": [ 0 ] })");
  ser_deser(R"({ "map": [ 1, 2 ] })");
  ser_deser(R"({ "map": null })");
  ser_deser(R"({ "map": true })");
  ser_deser(R"({ "map": false })");
  ser_deser(R"({ "map": 42 })");
  ser_deser(R"({ "map": 42.200000 })");
  ser_deser(R"([])");
  ser_deser(R"([ [] ])");
  ser_deser(R"([ 1, 2, { "hello": "world" } ])");

  auto dec_fail = [&dec](const string &s) {
    mem_input_stream c(s.c_str(), s.size());
    bool threw = false;
    try {
      load(c, dec);
    } catch (const err &e) {
      threw = true;
    }
    ELLIS_ASSERT(threw);
  };
  dec_fail("nil");
  dec_fail("01");
  dec_fail("-01");
  // TODO: this is supposed to fail.
  // dec_fail("10223372036854775808");  // too big
  return 0;
}
