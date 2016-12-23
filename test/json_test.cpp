#undef NDEBUG
#include <ellis/codec/json.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <sstream>


int main() {
  using namespace ellis;

  // set_system_log_prefilter(log_severity::DBUG);
  json_decoder dec;
  json_encoder enc;

  auto ser_deser = [&dec, &enc](const string & s) {
    ELLIS_LOG(NOTI, "===========================================");
    std::stringstream ss1;
    ss1 << s;
    ss1.flush();
    auto s1 = ss1.str();
    auto n = load(cpp_input_stream(ss1), dec);
    std::stringstream ss2;
    dump(n.get(), cpp_output_stream(ss2), enc);
    auto s2 = ss2.str();
    ELLIS_ASSERT_EQ(s1, s2);
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
  // TODO: enable this test after fix quoting inside strings
  // ser_deser(R"("s\"im\"ple")");
  // ser_deser(R"("sim\nple")");
  // ser_deser(R"("sim\u2388ple")");
  ser_deser(R"({  })");
  ser_deser(R"({ "map": {  } })");
  ser_deser(R"({ "map": [  ] })");
  ser_deser(R"({ "map": [ 0 ] })");
  ser_deser(R"({ "map": [ 1, 2 ] })");
  ser_deser(R"({ "map": null })");
  ser_deser(R"({ "map": true })");
  ser_deser(R"({ "map": false })");
  ser_deser(R"({ "map": 42 })");
  ser_deser(R"({ "map": 42.200000 })");
  ser_deser(R"([  ])");
  ser_deser(R"([ [  ] ])");
  ser_deser(R"([ 1, 2, { "hello": "world" } ])");
  // std::stringstream ss1;
  // ss1 << "[ 1, 2, { \"hello\": \"world\" } ]";
  // ss1.flush();
  // auto s1 = ss1.str();
  // auto n = load(cpp_input_stream(ss1), json_decoder());
  // std::stringstream ss2;
  // dump(n.get(), cpp_output_stream(ss2), json_encoder());
  // auto s2 = ss2.str();
  // ELLIS_ASSERT_EQ(s1, s2);
  auto dec_fail = [&dec](const string &s) {
    std::stringstream ss1;
    ss1 << s;
    ss1.flush();
    cpp_input_stream c(ss1);
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
