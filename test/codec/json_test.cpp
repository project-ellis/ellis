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

void check_give_back()
{
  using namespace ellis;
  const char *str = "2.5 15.5";
  size_t len = strlen(str);
  mem_input_stream mis(str, len);
  json_decoder dec;
  dec.reset();
  const byte *buf = nullptr;
  size_t buf_remain = 0;
  /* Should have gotten a buffer, thus success return. */
  ELLIS_ASSERT(mis.next_input_buf(&buf, &buf_remain));
  ELLIS_ASSERT_NOT_NULL(buf);
  ELLIS_ASSERT_EQ(buf_remain, len);
  /* Read the first value from the stream. */
  auto st = dec.consume_buffer(buf, &buf_remain);
  ELLIS_ASSERT_EQ(st.state(), stream_state::SUCCESS);
  auto val = st.extract_value();
  ELLIS_ASSERT_DBL_EQ(val->as_double(), 2.5);
  ELLIS_ASSERT_EQ(buf_remain, len - 4);
  /* Put back remaining bytes and read again. */
  mis.put_back(buf_remain);
  dec.reset();
  st = dec.consume_buffer(buf + (len - buf_remain), &buf_remain);
  ELLIS_ASSERT_EQ(buf_remain, 0UL);
  ELLIS_ASSERT_EQ(st.state(), stream_state::CONTINUE);
  st = dec.chop();
  ELLIS_ASSERT_EQ(st.state(), stream_state::SUCCESS);
  val = st.extract_value();
  ELLIS_ASSERT_EQ(val->as_double(), 15.5);
}

int main() {
  using namespace ellis;

  // set_system_log_prefilter(log_severity::DBUG);
  check_give_back();
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
