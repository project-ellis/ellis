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
#include <ellis/core/system.hpp>
#include <ellis_private/using.hpp>
#include <ellis/stream/file_input_stream.hpp>
#include <ellis/stream/file_output_stream.hpp>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void round_trip_test(const char *mem, size_t len)
{
  using namespace ellis;
  char tempfile[] = "/tmp/mytestXXXXXX";
  int tmpfd = mkstemp(tempfile);
  ELLIS_ASSERT_GTE(tmpfd, 0);
  close(tmpfd);

  /* Write to file. */
  {
    file_output_stream fos(tempfile);
    const char *mem_end = mem + len;
    for (const char *p = mem; p < mem_end; ) {
      byte *buf;
      size_t avail;
      ELLIS_ASSERT_TRUE(fos.next_output_buf(&buf, &avail));
      ELLIS_ASSERT_GT(avail, 0);
      size_t how_much = std::min((size_t)(mem_end - p), avail);
      memcpy(buf, p, how_much);
      fos.emit(how_much);
      p += how_much;
    }
  }

  /* Read from file. */
  file_input_stream fis(tempfile);
  std::ostringstream got;
  while (1) {
    byte *buf;
    size_t avail;
    if (! fis.next_input_buf((const byte **)&buf, &avail)) {
      break;
    }
    ELLIS_ASSERT_GT(avail, 0);
    got.write((const char *)buf, avail);
  }

  /* Compare recreated data. */
  string s = got.str();
  ELLIS_ASSERT_EQ(s.size(), len);
  ELLIS_ASSERT_MEM_EQ((const byte *)(s.data()), (const byte *)mem, len);

  unlink(tempfile);
}

void round_trip_test(const char *mem)
{
  round_trip_test(mem, strlen(mem));
}

int main() {
  using namespace ellis;
  round_trip_test("");
  round_trip_test("'");
  round_trip_test("0");
  round_trip_test("something\nhere\n");
  char buf[20000];
  for (size_t i = 0; i < sizeof(buf); ++i) {
    buf[i] = 'A' + (i % 13);
  }
  round_trip_test(buf, 1000);
  round_trip_test(buf, 4095);
  round_trip_test(buf, 4096);
  round_trip_test(buf, 4097);
  round_trip_test(buf, 20000);
  return 0;
}
