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
#include <cstring>
#include <ellis/codec/msgpack.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/emigration.hpp>
#include <ellis/core/immigration.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis_private/using.hpp>

using namespace ellis;

void ser_deser(
    msgpack_decoder &dec,
    msgpack_encoder &enc,
    const node &n)
{
  /* Arbitrarily chosen length; must be large enough to fit encoded results. */
  constexpr size_t max_len = 4096;

  /* Encode. */
  unique_ptr<byte[]> out = make_unique<byte[]>(max_len);
  size_t tmp = max_len;
  enc.reset(&n);
  enc.fill_buffer(out.get(), &tmp);
  ELLIS_ASSERT_LT(tmp, max_len);
  size_t diff = max_len - tmp;

  /* Now decode. */
  dec.reset();
  tmp = diff;
  auto status = dec.consume_buffer(out.get(), &tmp);
  ELLIS_ASSERT_EQ(status.state(), stream_state::SUCCESS);
  ELLIS_ASSERT_NULL(status.extract_error().get());
  ELLIS_ASSERT_EQ(tmp, 0);

  node decoded = std::move(*status.extract_value());
  ELLIS_ASSERT_EQ(n, decoded);
}

void onebyte_fail(msgpack_decoder &dec, byte b)
{
  dec.reset();
  const byte buf[] = { b };
  size_t count = sizeof(buf);
  auto status = dec.consume_buffer(buf, &count);
  ELLIS_ASSERT_EQ(status.state(), stream_state::ERROR);
  ELLIS_ASSERT_NOT_NULL(status.extract_error().get());
}

int main() {
  msgpack_decoder dec;
  msgpack_encoder enc;

  /* Buffer length is too small. */
  {
    dec.reset();
    const byte buf[] = { 0xa2, 0x68, 0x69 };
    size_t count = sizeof(buf) - 1;
    auto status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status.state(), stream_state::CONTINUE);
    ELLIS_ASSERT_NULL(status.extract_error().get());
    count = 1;
    status = dec.consume_buffer(buf + sizeof(buf) - 1, &count);
    node n = *status.extract_value();
    ELLIS_ASSERT_TRUE(n.is_type(type::U8STR));
    ELLIS_ASSERT_EQ(n, "hi");
  }

  /* Unsupported type bytes. */
  {
    /* Unused byte. */
    onebyte_fail(dec, 0xc1);

    /* Ext bytes. */
    onebyte_fail(dec, 0xc7);
    onebyte_fail(dec, 0xc8);
    onebyte_fail(dec, 0xc9);

    /* Uint64 byte. */
    onebyte_fail(dec, 0xcf);

    /* Fixext bytes. */
    onebyte_fail(dec, 0xd4);
    onebyte_fail(dec, 0xd5);
    onebyte_fail(dec, 0xd6);
    onebyte_fail(dec, 0xd7);
    onebyte_fail(dec, 0xd8);
  }

  /* Map key is not a string. */
  /* { 0: 1 } */
  {
    dec.reset();
    const byte buf[] = { 0x81, 0x0, 0x1 };
    size_t count = sizeof(buf);
    auto status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status.state(), stream_state::ERROR);
    ELLIS_ASSERT_NOT_NULL(status.extract_error().get());
  }

  /* 7 */
  {
    ser_deser(dec, enc, node(7));
  }

  /* 1589715871 */
  {
    ser_deser(dec, enc, node(1589715871));
  }

  /* -1589715871 */
  {
    ser_deser(dec, enc, node(-1589715871));
  }

  /* -7 */
  {
    ser_deser(dec, enc, node(-7));
  }

  /* 0.0 */
  {
    ser_deser(dec, enc, node(0.0));
  }

  /*
   * Interestingly, IEEE 754 allows for -0.0. Check if it comes out the same.
   */
  {
    ser_deser(dec, enc, node(-0.0));
  }

  /* 42.7 */
  {
    ser_deser(dec, enc, node(42.7));
  }

  /* -42.7 */
  {
    ser_deser(dec, enc, node(-42.7));
  }

  /* "" */
  {
    ser_deser(dec, enc, node(""));
  }

  /* "hi" */
  {
    ser_deser(dec, enc, node("hi"));
  }

  {
    /* [] */
    ser_deser(dec, enc, node(type::ARRAY));
  }


  {
    /* [0, -1, 2, -4] */
    node n = node(type::ARRAY);
    array_node &a = n.as_mutable_array();
    a.append(0);
    a.append(-1);
    a.append(2);
    a.append(-4);
    ser_deser(dec, enc, n);
  }

  {
    /* [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15] */
    node n = node(type::ARRAY);
    array_node &a = n.as_mutable_array();
    for (int64_t i = 0; i < 16; i++) {
      a.append(i);
    }
    ser_deser(dec, enc, n);
  }

  {
    /* { "compact": true, "schema": 0 } */
    node n = node(type::MAP);
    map_node &m = n.as_mutable_map();
    m.insert("compact", true);
    m.insert("schema", 0);
    ser_deser(dec, enc, n);
  }

  {
    /* { "key1": -4.7, "key2": -18 } */
    node n = node(type::MAP);
    map_node &m = n.as_mutable_map();
    m.insert("key1", -4.7);
    m.insert("key2", -18);
    ser_deser(dec, enc, n);
  }

  {
    /*
     * { "key0": 0,
     *   "key1": 1,
     *   "key2": 2,
     *   "key3": 3,
     *   "key4": 4,
     *   "key5": 5,
     *   "key6": 6,
     *   "key7": 7,
     *   "key8": 8,
     *   "key9": 9,
     *   "key10": 10,
     *   "key11": 11,
     *   "key12": 12,
     *   "key13": 13,
     *   "key14": 14,
     *   "key15": 15
     * }
     */
    node n = node(type::MAP);
    map_node &m = n.as_mutable_map();
    for (int64_t i = 0; i < 16; i++) {
      const string &key = ELLIS_SSTRING("key" << i);
      m.insert(key, i);
    }
    ser_deser(dec, enc, n);
  }

  {
    /*
     * [
     *   {"compact": true, "schema": 0},
     *   1,
     *   [2, 3, 4]
     * ]
     */
    node n = node(type::ARRAY);
    array_node &a = n.as_mutable_array();

    a.append(node(type::MAP));
    a[0].as_mutable_map().insert("compact", true);
    a[0].as_mutable_map().insert("schema", 0);
    a.append(1);
    a.append(node(type::ARRAY));
    a[2].as_mutable_array().append(2);
    a[2].as_mutable_array().append(3);
    a[2].as_mutable_array().append(4);

    ser_deser(dec, enc, n);
  }

  {
    /*
     * {
     *   "compact": true,
     *   "schema": [1, 2, 3]
     * }
     */
    node n = node(type::MAP);
    map_node &m = n.as_mutable_map();
    node other = node(type::ARRAY);
    other.as_mutable_array().append(1);
    other.as_mutable_array().append(2);
    other.as_mutable_array().append(3);
    m.insert("compact", true);
    m.insert("schema", other);

    ser_deser(dec, enc, n);
  }

  return 0;
}
