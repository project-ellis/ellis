#include <ellis/codec/msgpack.hpp>

#include <ellis/core/err.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/type.hpp>
#include <ellis/core/u8str_node.hpp>
#include <ellis_private/using.hpp>
#include <ellis_private/utility.hpp>
#include <endian.h>

/* TODO: The decoder currently decodes an entire node at a time, and if it fails
 * to do so, consumes no bytes. It should instead do the following:
 * - If it can produce a node, consume all bytes required to do so, and return
 *   SUCCESS.
 * - If it cannot produce a node, consume all the bytes given and return
 *   SUCCESS.
 * Practically speaking, this means the parsing state must be represented
 * explicitly so that any parsing routine can be interrupted without anything
 * breaking. We should probably use a stack-based parser in the manner that the
 * JSON codec does. The code should look something like this:
 *
 * In consume_buffer, repeatedly call a routine to parse a single char and
 * update the parse state. If that routine ever returns SUCCESS, then we have a
 * node to return. If instead it does not, then we just return CONTINUE and
 * consume all the bytes given.
 */

/* We do this instead of an if statement or a range map for performance. */
#define HEX_POS_FIXINT \
  0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: \
    case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: \
    case 0x0e: case 0x0f: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: \
    case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1b: \
    case 0x1c: case 0x1d: case 0x1e: case 0x1f: case 0x20: case 0x21: case 0x22: \
    case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: case 0x28: case 0x29: \
    case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: case 0x30: \
    case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: \
    case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: \
    case 0x3f: case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: \
    case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: \
    case 0x4d: case 0x4e: case 0x4f: case 0x50: case 0x51: case 0x52: case 0x53: \
    case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: \
    case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: case 0x60: case 0x61: \
    case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: \
    case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: \
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: \
    case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: \
    case 0x7e: case 0x7f

#define HEX_FIXMAP \
  0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: \
    case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: \
    case 0x8e: case 0x8f

#define HEX_FIXARRAY \
  0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: \
    case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: \
    case 0x9e: case 0x9f

#define HEX_FIXSTR \
  0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: \
    case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: \
    case 0xae: case 0xaf: case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: \
    case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: \
    case 0xbc: case 0xbd: case 0xbe: case 0xbf

#define HEX_NEG_FIXINT \
  0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: \
    case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: \
    case 0xee: case 0xef: case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: \
    case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: \
    case 0xfc: case 0xfd: case 0xfe: case 0xff

#define HEX_NIL 0xc0
#define HEX_UNUSED 0xc1
#define HEX_FALSE 0xc2
#define HEX_TRUE 0xc3
#define HEX_BIN_8 0xc4
#define HEX_BIN_16 0xc5
#define HEX_BIN_32 0xc6
#define HEX_EXT 0xc7: case 0xc8: case 0xc9
#define HEX_FLOAT_32 0xca
#define HEX_FLOAT_64 0xcb
#define HEX_UINT_8 0xcc
#define HEX_UINT_16 0xcd
#define HEX_UINT_32 0xce
#define HEX_UINT_64 0xcf
#define HEX_INT_8 0xd0
#define HEX_INT_16 0xd1
#define HEX_INT_32 0xd2
#define HEX_INT_64 0xd3
#define HEX_FIXEXT 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8
#define HEX_STR_8 0xd9
#define HEX_STR_16 0xda
#define HEX_STR_32 0xdb
#define HEX_ARRAY_16 0xdc
#define HEX_ARRAY_32 0xdd
#define HEX_MAP_16 0xde
#define HEX_MAP_32 0xdf


namespace ellis {


enum class msgpack_type {
  POS_FIXINT,
  FIXMAP,
  FIXARRAY,
  FIXSTR,
  NIL,
  FALSE,
  TRUE,
  BIN_8,
  BIN_16,
  BIN_32,
  /* ext not supported */
  FLOAT_32,
  FLOAT_64,
  UINT_8,
  UINT_16,
  UINT_32,
  /* uint64 not supported */
  INT_8,
  INT_16,
  INT_32,
  INT_64,
  /* fixext not supported */
  STR_8,
  STR_16,
  STR_32,
  ARRAY_16,
  ARRAY_32,
  MAP_16,
  MAP_32,
  NEG_FIXINT
};


/* Forward declarations. */
static node parse_node(const byte *buf, size_t *bytecount);

static std::string parse_str(const byte *buf, size_t *bytecount);
static std::string parse_str(
    msgpack_type type,
    const byte *buf,
    size_t *bytecount);


msgpack_type get_msgpack_type(byte b)
{
  switch (b) {
    case HEX_POS_FIXINT:
      return msgpack_type::POS_FIXINT;

    case HEX_FIXMAP:
      return msgpack_type::FIXMAP;

    case HEX_FIXARRAY:
      return msgpack_type::FIXARRAY;

    case HEX_FIXSTR:
      return msgpack_type::FIXSTR;

    case HEX_NIL:
      return msgpack_type::NIL;

    case HEX_UNUSED:
      THROW_ELLIS_ERR(PARSE_FAIL, "type byte is 0xc1, which can't be used");

    case HEX_FALSE:
      return msgpack_type::FALSE;

    case HEX_TRUE:
      return msgpack_type::TRUE;

    case HEX_BIN_8:
      return msgpack_type::BIN_8;

    case HEX_BIN_16:
      return msgpack_type::BIN_16;

    case HEX_BIN_32:
      return msgpack_type::BIN_32;

    case HEX_EXT:
      THROW_ELLIS_ERR(
          PARSE_FAIL, "type byte corresponds to an ext type, which is unsupported");

    case HEX_FLOAT_32:
      return msgpack_type::FLOAT_32;

    case HEX_FLOAT_64:
      return msgpack_type::FLOAT_64;

    case HEX_UINT_8:
      return msgpack_type::UINT_8;

    case HEX_UINT_16:
      return msgpack_type::UINT_16;

    case HEX_UINT_32:
      return msgpack_type::UINT_32;

    case HEX_UINT_64:
      THROW_ELLIS_ERR(
          PARSE_FAIL, "type byte corresponds to a uint64, which is unsupported");

    case HEX_INT_8:
      return msgpack_type::INT_8;

    case HEX_INT_16:
      return msgpack_type::INT_16;

    case HEX_INT_32:
      return msgpack_type::INT_32;

    case HEX_INT_64:
      return msgpack_type::INT_64;

    case HEX_FIXEXT:
      THROW_ELLIS_ERR(
          PARSE_FAIL, "type byte maps fixext, which is unsupported");

    case HEX_STR_8:
      return msgpack_type::STR_8;

    case HEX_STR_16:
      return msgpack_type::STR_16;

    case HEX_STR_32:
      return msgpack_type::STR_32;

    case HEX_ARRAY_16:
      return msgpack_type::ARRAY_16;

    case HEX_ARRAY_32:
      return msgpack_type::ARRAY_32;

    case HEX_MAP_16:
      return msgpack_type::MAP_16;

    case HEX_MAP_32:
      return msgpack_type::MAP_32;

    case HEX_NEG_FIXINT:
      return msgpack_type::NEG_FIXINT;
    default:
      ELLIS_ASSERT_UNREACHABLE();
  }
}


/*
 * These functions are used to map a block of memory from big-endian to the host
 * endianness. They are needed because msgpack numeric types are big-endian.
 */
template <typename T>
static inline T from_be(const byte *buf);

template <>
int8_t from_be(const byte *buf)
{
  return *buf;
}

template <>
int16_t from_be(const byte *buf)
{
  return be16toh(*((uint16_t *)buf));
}

template <>
int32_t from_be(const byte *buf)
{
  return be32toh(*((uint32_t *)buf));
}

template <>
int64_t from_be(const byte *buf)
{
  return be64toh(*((uint64_t *)buf));
}

template <>
uint8_t from_be(const byte *buf)
{
  return *buf;
}

template <>
uint16_t from_be(const byte *buf)
{
  return be16toh(*((uint16_t *)buf));
}

template <>
uint32_t from_be(const byte *buf)
{
  return be32toh(*((uint32_t *)buf));
}

template <>
float from_be(const byte *buf)
{
  return union_cast<uint32_t, float>(be32toh(*((uint32_t *)buf)));
}

template <>
double from_be(const byte *buf)
{
  return union_cast<uint64_t, double>(be64toh(*((uint64_t *)buf)));
}

class bytecount_insufficient : public std::exception {};

static inline void verify_expected_count(size_t bytecount, size_t expected)
{
  if (bytecount < expected) {
    /*
     * Not a real error, but used for simply getting back to consume_buffer.
     */
    throw bytecount_insufficient();
  }
}


template <typename T>
static void compute_lengths(
    const byte *buf,
    size_t bytecount,
    T *header_len,
    T *data_len)
{
  *header_len = 1 + sizeof(T);
  verify_expected_count(bytecount, *header_len);
  *data_len = from_be<T>(buf + 1);
  verify_expected_count(bytecount, *header_len + *data_len);
}


template <typename T>
static inline node _make_map_node(
    const byte *buf,
    size_t *bytecount,
    T header_len,
    T map_len)
{
  const byte *end = buf + *bytecount;
  *bytecount -= header_len;
  node map = node(type::MAP);
  for (T i = 0; i < map_len; i++) {
    const byte *pos = end - *bytecount;
    const string &key = parse_str(pos, bytecount);

    pos = end - *bytecount;
    node value = parse_node(pos, bytecount);

    map.as_mutable_map().insert(key, value);
  }

  return map;
}


template <typename T>
static inline node make_map_node(const byte *buf, size_t *bytecount)
{
  T header_len;
  T array_len;
  compute_lengths<T>(buf, *bytecount, &header_len, &array_len);
  return _make_map_node<T>(buf, bytecount, header_len, array_len);
}


template <typename T>
static inline node _make_array_node(
    const byte *buf,
    size_t *bytecount,
    T header_len,
    T array_len)
{
  const byte *end = buf + *bytecount;
  *bytecount -= header_len;
  node array = node(type::ARRAY);
  for (T i = 0; i < array_len; i++) {
    const byte *pos = end - *bytecount;
    node n = parse_node(pos, bytecount);
    array.as_mutable_array().append(std::move(n));
  }

  return array;
}


template <typename T>
static inline node make_array_node(const byte *buf, size_t *bytecount)
{
  T header_len;
  T array_len;
  compute_lengths(buf, *bytecount, &header_len, &array_len);
  return _make_array_node<T>(buf, bytecount, header_len, array_len);
}


template <typename T>
static inline node make_bin_node(const byte *buf, size_t *bytecount)
{
  T header_len;
  T data_len;
  compute_lengths(buf, *bytecount, &header_len, &data_len);
  *bytecount -= header_len + data_len;
  return node(buf + header_len, data_len);
}


template <typename T>
static inline node make_num_node(const byte *buf, size_t *bytecount)
{
  verify_expected_count(*bytecount, 1 + sizeof(T));
  *bytecount -= 1 + sizeof(T);
  return node(from_be<T>(buf + 1));
}

template <typename T>
static inline string _make_str(
  const byte *buf,
  size_t *bytecount,
  T header_len,
  T data_len)
{
  *bytecount -= header_len + data_len;
  return string((const char *)(buf + header_len), data_len);
}


template <typename T>
static inline string make_str(const byte *buf, size_t *bytecount)
{
  T header_len;
  T data_len;
  compute_lengths(buf, *bytecount, &header_len, &data_len);
  return _make_str<T>(buf, bytecount, header_len, data_len);
}


node parse_node(const byte *buf, size_t *bytecount)
{
  msgpack_type type = get_msgpack_type(*buf);
  switch (type) {
    /* Map */
    case msgpack_type::FIXMAP:
      {
        uint8_t data_len = *buf & 0x0f;
        verify_expected_count(*bytecount, 1 + data_len);
        return _make_map_node<uint8_t>(buf, bytecount, 1, data_len);
      }
    case msgpack_type::MAP_16:
      return make_map_node<uint16_t>(buf, bytecount);
    case msgpack_type::MAP_32:
      return make_map_node<uint32_t>(buf, bytecount);

    /* Array */
    case msgpack_type::FIXARRAY:
      {
        uint8_t data_len = *buf & 0x0f;
        verify_expected_count(*bytecount, 1 + data_len);
        return _make_array_node<uint8_t>(buf, bytecount, 1, data_len);
      }
    case msgpack_type::ARRAY_16:
      return make_array_node<uint16_t>(buf, bytecount);
    case msgpack_type::ARRAY_32:
      return make_array_node<uint32_t>(buf, bytecount);

    /* String */
    case msgpack_type::FIXSTR:
    case msgpack_type::STR_8:
    case msgpack_type::STR_16:
    case msgpack_type::STR_32:
      return node(parse_str(type, buf, bytecount));

    /* Binary */
    case msgpack_type::BIN_8:
      return make_bin_node<uint8_t>(buf, bytecount);
    case msgpack_type::BIN_16:
      return make_bin_node<uint16_t>(buf, bytecount);
    case msgpack_type::BIN_32:
      return make_bin_node<uint32_t>(buf, bytecount);

    /* Numeric */
    case msgpack_type::FLOAT_32:
      return make_num_node<float>(buf, bytecount);
    case msgpack_type::FLOAT_64:
      return make_num_node<double>(buf, bytecount);

    case msgpack_type::UINT_8:
      return make_num_node<uint8_t>(buf, bytecount);
    case msgpack_type::UINT_16:
      return make_num_node<uint16_t>(buf, bytecount);
    case msgpack_type::UINT_32:
      return make_num_node<uint32_t>(buf, bytecount);

    case msgpack_type::INT_8:
      return make_num_node<int8_t>(buf, bytecount);
    case msgpack_type::INT_16:
      return make_num_node<int16_t>(buf, bytecount);
    case msgpack_type::INT_32:
      return make_num_node<int32_t>(buf, bytecount);
    case msgpack_type::INT_64:
      return make_num_node<int64_t>(buf, bytecount);

    case msgpack_type::POS_FIXINT:
      *bytecount -= 1;
      return node(static_cast<uint8_t>(*buf));
    case msgpack_type::NEG_FIXINT:
      *bytecount -= 1;
      return node(static_cast<int8_t>(*buf));

    /* Singletons */
    case msgpack_type::NIL:
      *bytecount -= 1;
      return node(ellis::type::NIL);
    case msgpack_type::FALSE:
      *bytecount -= 1;
      return node(false);
    case msgpack_type::TRUE:
      *bytecount -= 1;
      return node(true);
  }
  ELLIS_ASSERT_UNREACHABLE();
}


msgpack_decoder::msgpack_decoder()
{
}


string parse_str(const byte *buf, size_t *bytecount)
{
  msgpack_type type = get_msgpack_type(*buf);
  return parse_str(type, buf, bytecount);
}


string parse_str(msgpack_type type, const byte *buf, size_t *bytecount)
{
  switch (type) {
    case msgpack_type::FIXSTR:
      {
        uint8_t data_len = *buf & 0x1f;
        verify_expected_count(*bytecount, 1 + data_len);
        return _make_str<uint8_t>(buf, bytecount, 1, data_len);
      }
    case msgpack_type::STR_8:
      return make_str<uint8_t>(buf, bytecount);
    case msgpack_type::STR_16:
      return make_str<uint16_t>(buf, bytecount);
    case msgpack_type::STR_32:
      return make_str<uint32_t>(buf, bytecount);
    default:
      THROW_ELLIS_ERR(PARSE_FAIL, "Parsed node is not a string");
  }
}


/*  ____                     _
 * |  _ \  ___  ___ ___   __| | ___ _ __
 * | | | |/ _ \/ __/ _ \ / _` |/ _ \ '__|
 * | |_| |  __/ (_| (_) | (_| |  __/ |
 * |____/ \___|\___\___/ \__,_|\___|_|
 *
 */


node_progress msgpack_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  try {
    unique_ptr<node> n = make_unique<node>(parse_node(buf, bytecount));
    return node_progress(std::move(n));
  }
  catch (const bytecount_insufficient &) {
    return node_progress(stream_state::CONTINUE);
  }
  catch (const err &e) {
    return node_progress(make_unique<err>(e));
  }
}


void msgpack_decoder::reset()
{
}


node_progress msgpack_decoder::chop()
{
  return node_progress(MAKE_UNIQUE_ELLIS_ERR(PARSE_FAIL,
        "Cannot chop a msgpack decoder node"));
}


/*  _____                     _
 * | ____|_ __   ___ ___   __| | ___ _ __
 * |  _| | '_ \ / __/ _ \ / _` |/ _ \ '__|
 * | |___| | | | (_| (_) | (_| |  __/ |
 * |_____|_| |_|\___\___/ \__,_|\___|_|
 *
 */


msgpack_encoder::msgpack_encoder()
{
}


progress msgpack_encoder::fill_buffer(
    UNUSED byte *buf,
    UNUSED size_t *bytecount)
{
  /* TODO */
  ELLIS_ASSERT_UNREACHABLE();
}


void msgpack_encoder::reset(UNUSED const node *new_node)
{
}


}  /* namespace ellis */
