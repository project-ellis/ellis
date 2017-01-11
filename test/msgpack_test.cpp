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
    const byte *buf,
    size_t len,
    const byte *expected = nullptr,
    size_t expected_len = 0) {
    ELLIS_LOG(NOTI, "===========================================");
    dec.reset();
    size_t tmp = len;
    auto status = dec.consume_buffer(buf, &tmp);
    ELLIS_ASSERT_EQ(status.state(), stream_state::SUCCESS);
    ELLIS_ASSERT_NULL(status.extract_error().get());

    node n = *status.extract_value();
    unique_ptr<byte[]> out = make_unique<byte[]>(len);
    tmp = len;
    enc.reset(&n);
    enc.fill_buffer(out.get(), &tmp);
    if (expected == nullptr) {
      expected = buf;
    }
    ELLIS_ASSERT_MEM_EQ(expected, out.get(), expected_len);
}

void onebyte_fail(msgpack_decoder &dec, byte b)
{
  const byte buf[] = { b };
  size_t count = sizeof(buf);
  auto status = dec.consume_buffer(buf, &count);
  ELLIS_ASSERT_EQ(status.state(), stream_state::ERROR);
  ELLIS_ASSERT_NOT_NULL(status.extract_error().get());
}

int main() {
  msgpack_decoder dec;
  msgpack_encoder enc;

  /*
   * TODO: This test is wrong. After the first call returns CONTINUE, *bytecount
   * should be 0, and the caller should call in with the third byte.
   */

  /* Buffer length is too small. */
  {
    const byte buf[] = { 0xa2, 0x68, 0x69 };
    size_t count = sizeof(buf) - 1;
    auto status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status.state(), stream_state::CONTINUE);
    ELLIS_ASSERT_NULL(status.extract_error().get());
    count = sizeof(buf);
    status = dec.consume_buffer(buf, &count);
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
    const byte buf[] = { 0x81, 0x0, 0x1 };
    size_t count = sizeof(buf);
    auto status = dec.consume_buffer(buf, &count);
    ELLIS_ASSERT_EQ(status.state(), stream_state::ERROR);
    ELLIS_ASSERT_NOT_NULL(status.extract_error().get());
  }

  /* 7 */
  {
    const byte buf[] = { 0x7 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* 1589715871 */
  {
    const byte buf[] = { 0xce, 0x5e, 0xc1, 0x23, 0x9f };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* -1589715871 */
  {
    const byte buf[] = { 0xd2, 0xa1, 0x3e, 0xdc, 0x61 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* -7 */
  {
    const byte buf[] = { 0xf9 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* 0.0 */
  {
    const byte buf[] = { 0xcb, 0, 0, 0, 0, 0, 0, 0, 0 };
    const byte expected[] = { 0xca, 0, 0, 0, 0 };
    ser_deser(dec, enc, buf, sizeof(buf), expected, sizeof(expected));
  }

  /*
   * Interestingly, IEEE 754 allows for -0.0. Check if it comes out the same.
   */
  {
    const byte buf[] = { 0xcb, 0x80, 0, 0, 0, 0, 0, 0, 0 };
    const byte expected[] = { 0xca, 0, 0, 0, 0x80 };
    ser_deser(dec, enc, buf, sizeof(buf), expected, sizeof(expected));
  }

  /* 42.7 */
  {
    const byte buf[] = { 0xcb, 0x40, 0x45, 0x59, 0x99, 0x99, 0x99, 0x99, 0x9a };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* -42.7 */
  {
    const byte buf[] = { 0xcb, 0xc0, 0x45, 0x59, 0x99, 0x99, 0x99, 0x99, 0x9a };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    const byte buf[] = { 0xa0 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  /* "hi" */
  {
    const byte buf[] = { 0xa2, 0x68, 0x69 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /* [0, -1, 2, -4] */
    const byte buf[] = { 0x94, 0x00, 0xff, 0x02, 0xfc };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /* [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15] */
    const byte buf[] = {
      0xdc, 0x00, 0x10, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
      0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /* { "compact": true, "schema": 0 } */
    const byte buf[] = {
      0x82, 0xa7, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74,
      0xc3, 0xa6, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x00 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /* { "key1": -4.7, "key2": -18 } */
    const byte buf[] = {
      0x82, 0xa4, 0x6b, 0x65, 0x79, 0x31, 0xcb, 0xc0, 0x12, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcd, 0xa4, 0x6b, 0x65, 0x79, 0x32, 0xee };
    ser_deser(dec, enc, buf, sizeof(buf));
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
    const byte buf[] = {
      0xde, 0x00, 0x10, 0xa4, 0x6b, 0x65, 0x79, 0x30, 0x00, 0xa4, 0x6b, 0x65,
      0x79, 0x31, 0x01, 0xa4, 0x6b, 0x65, 0x79, 0x32, 0x02, 0xa4, 0x6b, 0x65,
      0x79, 0x33, 0x03, 0xa4, 0x6b, 0x65, 0x79, 0x34, 0x04, 0xa4, 0x6b, 0x65,
      0x79, 0x35, 0x05, 0xa4, 0x6b, 0x65, 0x79, 0x36, 0x06, 0xa4, 0x6b, 0x65,
      0x79, 0x37, 0x07, 0xa4, 0x6b, 0x65, 0x79, 0x38, 0x08, 0xa4, 0x6b, 0x65,
      0x79, 0x39, 0x09, 0xa5, 0x6b, 0x65, 0x79, 0x31, 0x30, 0x0a, 0xa5, 0x6b,
      0x65, 0x79, 0x31, 0x31, 0x0b, 0xa5, 0x6b, 0x65, 0x79, 0x31, 0x32, 0x0c,
      0xa5, 0x6b, 0x65, 0x79, 0x31, 0x33, 0x0d, 0xa5, 0x6b, 0x65, 0x79, 0x31,
      0x34, 0x0e, 0xa5, 0x6b, 0x65, 0x79, 0x31, 0x35, 0x0f };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /*
     * [
     *   {"compact": true, "schema": 0},
     *   1,
     *   [2, 3, 4]
     * ]
     */
    const byte buf[] = {
      0x93, 0x82, 0xa7, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74, 0xc3, 0xa6,
      0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x00, 0x01, 0x93, 0x02, 0x03, 0x04 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  {
    /*
     * {
     *   "compact": true,
     *   "schema": [1, 2, 3]
     * }
     */
    const byte buf[] = {
      0x82, 0xa7, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x63, 0x74, 0xc3, 0xa6, 0x73,
      0x63, 0x68, 0x65, 0x6d, 0x61, 0x93, 0x01, 0x02, 0x03 };
    ser_deser(dec, enc, buf, sizeof(buf));
  }

  return 0;
}
