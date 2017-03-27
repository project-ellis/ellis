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
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/system.hpp>
#include <ellis/codec/delimited_text.hpp>
#include <ellis_private/using.hpp>
#include <ellis/stream/cpp_input_stream.hpp>
#include <ellis/stream/cpp_output_stream.hpp>
#include <sstream>

// TODO: Use mkstemp to create a temporary file, write to the fd, read from it,
// and close.

int main() {
  using namespace ellis;
  //auto n = load(mem_input_stream("[1,2,3]"), json_decoder());
  //dump(n, file_output_stream("/tmp/lame.json"), json_encoder());
  //file_input_stream is("/tmp/lame.json");
  //json_decoder jd;
  //unique_ptr<node> n1(load(is, jd));
  //unique_ptr<node> n2(load(is, jd));
  std::stringstream ss1;
  ss1 << "one" << std::endl << "two" << std::endl << "three" << std::endl;
  auto s1 = ss1.str();
  auto n = load(cpp_input_stream(ss1), delimited_text_decoder());
  std::stringstream ss2;
  dump(n.get(), cpp_output_stream(ss2), delimited_text_encoder());
  auto s2 = ss2.str();
  ELLIS_ASSERT_EQ(s1, s2);
  return 0;
}
