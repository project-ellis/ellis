#include <ellis/codec/msgpack.hpp>

#include <cfloat>
#include <cstdint>
#include <cstring>
#include <ellis/core/err.hpp>
#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/type.hpp>
#include <ellis/core/u8str_node.hpp>
#include <ellis_private/using.hpp>
#include <ellis_private/utility.hpp>
#include <endian.h>

/* TODO: The encoder currently encodes an entire node at a time, thus copying
 * all the  memory given, which is wasteful. It should instead explicitly
 * represent its state in order to encode only as much of the buffer as is given
 * and do minimal copying.
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
#define HEX_BIN8 0xc4
#define HEX_BIN16 0xc5
#define HEX_BIN32 0xc6
#define HEX_EXT 0xc7: case 0xc8: case 0xc9
#define HEX_FLOAT32 0xca
#define HEX_FLOAT64 0xcb
#define HEX_UINT8 0xcc
#define HEX_UINT16 0xcd
#define HEX_UINT32 0xce
#define HEX_UINT64 0xcf
#define HEX_INT8 0xd0
#define HEX_INT16 0xd1
#define HEX_INT32 0xd2
#define HEX_INT64 0xd3
#define HEX_FIXEXT 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8
#define HEX_STR8 0xd9
#define HEX_STR16 0xda
#define HEX_STR32 0xdb
#define HEX_ARRAY16 0xdc
#define HEX_ARRAY32 0xdd
#define HEX_MAP16 0xde
#define HEX_MAP32 0xdf

namespace ellis {


enum class msgpack_type {
  NIL,
  FALSE,
  TRUE,
  POS_FIXINT,
  NEG_FIXINT,
  FIXMAP,
  FIXARRAY,
  FIXSTR,
  BIN8,
  BIN16,
  BIN32,
  /* ext not supported */
  FLOAT32,
  FLOAT64,
  UINT8,
  UINT16,
  UINT32,
  /* uint64 not supported */
  INT8,
  INT16,
  INT32,
  INT64,
  /* fixext not supported */
  STR8,
  STR16,
  STR32,
  ARRAY16,
  ARRAY32,
  MAP16,
  MAP32
};


parse_ctx::parse_ctx() :
  state(parse_state::UNDEFINED)
{
}


// TODO: doc
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
      THROW_ELLIS_ERR(PARSE_FAIL,
          "type byte is 0xc1, which can't be used");

    case HEX_FALSE:
      return msgpack_type::FALSE;

    case HEX_TRUE:
      return msgpack_type::TRUE;

    case HEX_BIN8:
      return msgpack_type::BIN8;

    case HEX_BIN16:
      return msgpack_type::BIN16;

    case HEX_BIN32:
      return msgpack_type::BIN32;

    case HEX_EXT:
      THROW_ELLIS_ERR(TRANSLATE_FAIL,
          "type byte corresponds to an ext type, which is unsupported");

    case HEX_FLOAT32:
      return msgpack_type::FLOAT32;

    case HEX_FLOAT64:
      return msgpack_type::FLOAT64;

    case HEX_UINT8:
      return msgpack_type::UINT8;

    case HEX_UINT16:
      return msgpack_type::UINT16;

    case HEX_UINT32:
      return msgpack_type::UINT32;

    case HEX_UINT64:
      THROW_ELLIS_ERR(TRANSLATE_FAIL,
          "type byte corresponds to a uint64, which is unsupported");

    case HEX_INT8:
      return msgpack_type::INT8;

    case HEX_INT16:
      return msgpack_type::INT16;

    case HEX_INT32:
      return msgpack_type::INT32;

    case HEX_INT64:
      return msgpack_type::INT64;

    case HEX_FIXEXT:
      THROW_ELLIS_ERR(TRANSLATE_FAIL,
          "type byte maps fixext, which is unsupported");

    case HEX_STR8:
      return msgpack_type::STR8;

    case HEX_STR16:
      return msgpack_type::STR16;

    case HEX_STR32:
      return msgpack_type::STR32;

    case HEX_ARRAY16:
      return msgpack_type::ARRAY16;

    case HEX_ARRAY32:
      return msgpack_type::ARRAY32;

    case HEX_MAP16:
      return msgpack_type::MAP16;

    case HEX_MAP32:
      return msgpack_type::MAP32;

    case HEX_NEG_FIXINT:
      return msgpack_type::NEG_FIXINT;
    default:
      ELLIS_ASSERT_UNREACHABLE();
  }
}


template <typename T>
static inline
T accum_be(T num, byte b)
{
  return (num << 8) | b;
}


void accum_header(parse_ctx & ctx, byte b, parse_state next)
{
  ctx.data_len = accum_be(ctx.data_len, b);
  --ctx.header_len;
  if (ctx.header_len == 0) {
    ctx.state = next;
  }
}


void init_vec_data(parse_ctx & ctx, size_t len)
{
  ctx.buf = make_unique<vector<byte>>();
  ctx.buf->reserve(len);
}


void msgpack_decoder::accum_str_header(parse_ctx & ctx, byte b)
{
  accum_header(ctx, b, parse_state::STR_DATA);
  if (ctx.state == parse_state::STR_DATA) {
    init_vec_data(ctx, ctx.data_len);
  }
}


void msgpack_decoder::accum_map_key_header(parse_ctx & ctx, byte b)
{
  accum_header(ctx, b, parse_state::MAP_KEY_DATA);
  if (ctx.state == parse_state::MAP_KEY_DATA) {
    /* We store the map length separately from data length so we don't
     * clobber it when we start parsing the data length of each map key. */
    init_vec_data(ctx, ctx.data_len);
    ctx.map_len = ctx.data_len;
  }
}


void msgpack_decoder::accum_bin_header(parse_ctx & ctx, byte b)
{
  accum_header(ctx, b, parse_state::BIN_DATA);
  if (ctx.state == parse_state::BIN_DATA) {
    init_vec_data(ctx, ctx.data_len);
  }
}


node_progress msgpack_decoder::accum_arr_header(parse_ctx & ctx, byte b)
{
  accum_header(ctx, b, parse_state::ARRAY_DATA);
  if (ctx.state == parse_state::ARRAY_DATA) {
    /* Empty array. */
    unique_ptr<node> n = make_unique<node>(ellis::type::ARRAY);
    if (ctx.data_len == 0) {
      return node_progress(std::move(n));
    }
    else {
      ctx.node = std::move(n);
      m_parse_stack.emplace_back();
    }
  }
  return node_progress(stream_state::CONTINUE);
}


node_progress msgpack_decoder::accum_map_header(parse_ctx & ctx, byte b)
{
  const parse_state next = parse_state::MAP_KEY_TYPE;
  accum_header(ctx, b, next);
  if (ctx.state == next) {
    /* We store the map length separately from data length so we don't
     * clobber it when we start parsing the data length of each map key. */
    ctx.map_len = ctx.data_len;
    unique_ptr<node> n = make_unique<node>(ellis::type::MAP);
    if (ctx.map_len == 0) {
      return node_progress(std::move(n));
    }
    else {
      ctx.node = std::move(n);
    }
  }
  return node_progress(stream_state::CONTINUE);
}


template <typename T>
node_progress accum_num_data(parse_ctx & ctx, byte b)
{
  ctx.data = accum_be(ctx.data, b);
  --ctx.data_len;
  if (ctx.data_len == 0) {
    unique_ptr<node> n = make_unique<node>(
        union_cast<decltype(ctx.data), T>(ctx.data));
    return node_progress(std::move(n));
  }
  return node_progress(stream_state::CONTINUE);
}


bool accum_str(parse_ctx & ctx, byte b)
{
  ctx.buf->push_back(b);
  --ctx.data_len;
  if (ctx.data_len == 0) {
    return true;
  }
  return false;
}


node_progress accum_str_node(parse_ctx & ctx, byte b)
{
  bool done = accum_str(ctx, b);
  if (done) {
    /* TODO: Should not have to copy into a string */
    const string s((char *) ctx.buf->data(), ctx.buf->size());
    unique_ptr<node> n = make_unique<node>(std::move(s));
    return node_progress(std::move(n));
  }

  return node_progress(stream_state::CONTINUE);
}


node_progress accum_bin(parse_ctx & ctx, byte b)
{
  ctx.buf->push_back(b);
  --ctx.data_len;
  if (ctx.data_len == 0) {
    unique_ptr<node> n = make_unique<node>(ctx.buf->data(), ctx.buf->size());
    return node_progress(std::move(n));
  }
  return node_progress(stream_state::CONTINUE);
}


bool type_is_string(msgpack_type type)
{
  switch (type) {
    case msgpack_type::FIXSTR:
    case msgpack_type::STR8:
    case msgpack_type::STR16:
    case msgpack_type::STR32:
      return true;
    default:
      return false;
  }
}


uint_fast32_t get_fixstr_length(byte b) {
  return b & 0x1f;
}


node_progress progmore()
{
  return node_progress(stream_state::CONTINUE);
}


node_progress msgpack_decoder::handle_type(parse_ctx &ctx, byte b)
{
  msgpack_type type = get_msgpack_type(b);
  switch (type) {
    case msgpack_type::NIL:
      return node_progress(make_unique<node>(ellis::type::NIL));

    case msgpack_type::FALSE:
      return node_progress(make_unique<node>(false));

    case msgpack_type::TRUE:
      return node_progress(make_unique<node>(true));

    case msgpack_type::POS_FIXINT:
      return node_progress(make_unique<node>(static_cast<uint8_t>(b)));

    case msgpack_type::NEG_FIXINT:
      return node_progress(make_unique<node>(static_cast<int8_t>(b)));

    /* TODO: refactor so the FIX* versions don't require any duplication. */

    case msgpack_type::FIXMAP:
      ctx.state = parse_state::MAP_KEY_TYPE;
      ctx.map_len = b & 0x0f;
      {
        unique_ptr<node> n = make_unique<node>(ellis::type::MAP);
        if (ctx.map_len == 0) {
          return node_progress(std::move(n));
        }
        else {
          ctx.node = std::move(n);
        }
      }
      return progmore();

    case msgpack_type::FIXARRAY:
      ctx.state = parse_state::ARRAY_DATA;
      ctx.data_len = b & 0x0f;
      {
        unique_ptr<node> n = make_unique<node>(ellis::type::ARRAY);
        if (ctx.data_len == 0) {
          return node_progress(std::move(n));
        }
        else {
          ctx.node = std::move(n);
          m_parse_stack.emplace_back();
        }
      }
      return progmore();

    case msgpack_type::FIXSTR:
      ctx.state = parse_state::STR_DATA;
      ctx.data_len = get_fixstr_length(b);
      if (ctx.data_len == 0) {
        return node_progress(make_unique<node>(""));
      }
      init_vec_data(ctx, ctx.data_len);
      return progmore();

    case msgpack_type::BIN8:
      ctx.state = parse_state::BIN_DATA;
      ctx.header_len = sizeof(uint8_t);
      return progmore();

    case msgpack_type::BIN16:
      ctx.state = parse_state::BIN_DATA;
      ctx.header_len = sizeof(uint16_t);
      return progmore();

    case msgpack_type::BIN32:
      ctx.state = parse_state::BIN_DATA;
      ctx.header_len = sizeof(uint32_t);
      return progmore();

    case msgpack_type::FLOAT32:
      ctx.state = parse_state::FLOAT32_DATA;
      ctx.data_len = sizeof(float);
      ctx.data = 0;
      return progmore();

    case msgpack_type::FLOAT64:
      ctx.state = parse_state::FLOAT64_DATA;
      ctx.data_len = sizeof(double);
      ctx.data = 0;
      return progmore();

    case msgpack_type::UINT8:
      ctx.state = parse_state::UINT8_DATA;
      ctx.data_len = sizeof(uint8_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::UINT16:
      ctx.state = parse_state::UINT16_DATA;
      ctx.data_len = sizeof(uint16_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::UINT32:
      ctx.state = parse_state::UINT32_DATA;
      ctx.data_len = sizeof(uint32_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::INT8:
      ctx.state = parse_state::INT8_DATA;
      ctx.data_len = sizeof(int8_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::INT16:
      ctx.state = parse_state::INT16_DATA;
      ctx.data_len = sizeof(int16_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::INT32:
      ctx.state = parse_state::INT32_DATA;
      ctx.data_len = sizeof(int32_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::INT64:
      ctx.state = parse_state::INT64_DATA;
      ctx.data_len = sizeof(int64_t);
      ctx.data = 0;
      return progmore();

    case msgpack_type::STR8:
      ctx.state = parse_state::STR_HEADER;
      ctx.header_len = sizeof(uint8_t);
      return progmore();

    case msgpack_type::STR16:
      ctx.state = parse_state::STR_HEADER;
      ctx.header_len = sizeof(uint16_t);
      return progmore();

    case msgpack_type::STR32:
      ctx.state = parse_state::STR_HEADER;
      ctx.header_len = sizeof(uint32_t);
      return progmore();

    case msgpack_type::ARRAY16:
      ctx.state = parse_state::ARRAY_HEADER;
      ctx.header_len = sizeof(uint16_t);
      return progmore();

    case msgpack_type::ARRAY32:
      ctx.state = parse_state::ARRAY_HEADER;
      ctx.header_len = sizeof(uint32_t);
      return progmore();

    case msgpack_type::MAP16:
      ctx.state = parse_state::MAP_HEADER;
      ctx.header_len = sizeof(uint16_t);
      return progmore();

    case msgpack_type::MAP32:
      ctx.state = parse_state::MAP_HEADER;
      ctx.header_len = sizeof(uint32_t);
      return progmore();
  }
  ELLIS_ASSERT_UNREACHABLE();
}


node_progress msgpack_decoder::parse_byte(byte b)
{
  /* TODO: Don't copy if the entire buffer is available in memory (IOW, skip buf
   * in that case. */

  /*
   * Reserve storage so that the reference we make here does not get
   * invalidated if we push another context onto the stack. Reserve twice the
   * amount we need so we get amortized O(1) time in copying elements.
   */
  m_parse_stack.reserve(m_parse_stack.size() * 2);

  parse_ctx &ctx = m_parse_stack.back();
  node_progress prog(stream_state::CONTINUE);
  switch (ctx.state) {
    case parse_state::UNDEFINED:
      prog = handle_type(ctx, b);
      break;

    case parse_state::STR_HEADER:
      accum_str_header(ctx, b);
      break;

    case parse_state::BIN_HEADER:
      accum_bin_header(ctx, b);
      break;

    case parse_state::ARRAY_HEADER:
      prog = accum_arr_header(ctx, b);
      break;

    case parse_state::MAP_HEADER:
      prog = accum_map_header(ctx, b);
      break;

    case parse_state::UINT8_DATA:
      prog = node_progress(make_unique<node>(static_cast<int64_t>(b)));
      break;

    case parse_state::UINT16_DATA:
      prog = accum_num_data<uint16_t>(ctx, b);
      break;

    case parse_state::UINT32_DATA:
      prog = accum_num_data<uint32_t>(ctx, b);
      break;

    case parse_state::INT8_DATA:
      prog = accum_num_data<int8_t>(ctx, b);
      break;

    case parse_state::INT16_DATA:
      prog = accum_num_data<int16_t>(ctx, b);
      break;

    case parse_state::INT32_DATA:
      prog = accum_num_data<int32_t>(ctx, b);
      break;

    case parse_state::INT64_DATA:
      prog = accum_num_data<int64_t>(ctx, b);
      break;

    case parse_state::FLOAT32_DATA:
      prog = accum_num_data<float>(ctx, b);
      break;

    case parse_state::FLOAT64_DATA:
      prog = accum_num_data<double>(ctx, b);
      break;

    case parse_state::STR_DATA:
      prog = accum_str_node(ctx, b);
      break;

    case parse_state::BIN_DATA:
      prog = accum_bin(ctx, b);
      break;

    case parse_state::ARRAY_DATA:
      ELLIS_ASSERT_UNREACHABLE();
      break;

    case parse_state::MAP_KEY_TYPE:
      {
        msgpack_type type = get_msgpack_type(b);
        if (!type_is_string(type)) {
          THROW_ELLIS_ERR(TRANSLATE_FAIL, "Map key must be a string");
        }
        if (type != msgpack_type::FIXSTR) {
          ctx.state = parse_state::MAP_KEY_HEADER;
        }
        else {
          ctx.data_len = get_fixstr_length(b);
          ctx.state = parse_state::MAP_KEY_DATA;
          init_vec_data(ctx, ctx.data_len);
        }
      }
      break;

    case parse_state::MAP_KEY_HEADER:
      accum_map_key_header(ctx, b);
      break;

    case parse_state::MAP_KEY_DATA:
      {
        bool done = accum_str(ctx, b);
        if (done) {
          ctx.state = parse_state::MAP_VALUE_DATA;
          m_parse_stack.emplace_back();
        }
      }
      break;

    case parse_state::MAP_VALUE_DATA:
      ELLIS_ASSERT_UNREACHABLE();
      break;

    case parse_state::COMPLETE:
      ELLIS_ASSERT_UNREACHABLE();
  }

  if (prog.state() != stream_state::SUCCESS) {
    return prog;
  }
  ctx.node = std::move(prog.extract_value());
  ctx.state = parse_state::COMPLETE;

  /*
   * Reduce the stack by absorbing the top of the stack into parent containers
   * lower in the stack.
   */

  while (m_parse_stack.size() > 1) {
    parse_ctx &top = m_parse_stack.back();
    if (top.state != parse_state::COMPLETE) {
      /* Node is not complete. */
      return node_progress(stream_state::CONTINUE);
    }

    unique_ptr<node> n = std::move(top.node);
    m_parse_stack.pop_back();
    parse_ctx &parent = m_parse_stack.back();
    if (parent.node->get_type() == type::ARRAY) {
      parent.node->as_mutable_array().append(*n);
      --parent.data_len;
      if (parent.data_len > 0) {
        m_parse_stack.emplace_back();
      }
      else {
        parent.state = parse_state::COMPLETE;
      }
    }
    else if (parent.node->get_type() == type::MAP) {
      /* TODO: Should not have to copy into a string */
      const string key((char *) parent.buf->data(), parent.buf->size());
      parent.node->as_mutable_map().insert(std::move(key).c_str(), *n);
      --parent.map_len;
      if (parent.map_len > 0) {
        parent.state = parse_state::MAP_KEY_TYPE;
      }
      else {
        parent.state = parse_state::COMPLETE;
      }
    }
    else {
      THROW_ELLIS_ERR(
        PARSE_FAIL, "Parsed two primitives in a row not inside a container");
    }
  }

  parse_ctx &top = m_parse_stack.back();
  if (top.state != parse_state::COMPLETE) {
    /* Node is not complete. */
    return node_progress(stream_state::CONTINUE);
  }

  unique_ptr<node> n = std::move(top.node);
  m_parse_stack.pop_back();
  return node_progress(std::move(n));
}


msgpack_decoder::msgpack_decoder()
{
  msgpack_decoder::reset();
}


node_progress msgpack_decoder::consume_buffer(
    const byte *buf,
    size_t *bytecount)
{
  const byte *end = buf + *bytecount;
  for (const byte *p = buf; p < end; p++) {
    try {
      node_progress st = parse_byte(*p);
      if (st.state() == stream_state::SUCCESS
          || st.state() == stream_state::ERROR) {
        *bytecount = end - p - 1;
        return st;
      }
    }
    catch (const err &e) {
      return node_progress(make_unique<err>(e));
    }
  }
  *bytecount = 0;
  return node_progress(stream_state::CONTINUE);
}


void msgpack_decoder::reset()
{
  m_parse_stack.clear();
  m_parse_stack.emplace_back();
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


msgpack_encoder::msgpack_encoder() :
  m_pos(0),
  m_end(0)
{
}


/* These functions are used for pushing a value into memory as big-endian. */

void msgpack_encoder::_push_be(uint8_t val)
{
  m_buf.push_back(val);
}

void msgpack_encoder::_push_be(uint16_t val)
{
  // TODO: macro to extract nth byte from LSB.
  m_buf.push_back((val & 0xff00) >> 8);
  m_buf.push_back((val & 0x00ff) >> 0);
}

void msgpack_encoder::_push_be(uint32_t val)
{
  m_buf.push_back((val & 0xff000000) >> 24);
  m_buf.push_back((val & 0x00ff0000) >> 16);
  m_buf.push_back((val & 0x0000ff00) >> 8);
  m_buf.push_back((val & 0x000000ff) >> 0);
}

void msgpack_encoder::_push_be(uint64_t val)
{
  m_buf.push_back((val & 0xff00000000000000) >> 56);
  m_buf.push_back((val & 0x00ff000000000000) >> 48);
  m_buf.push_back((val & 0x0000ff0000000000) >> 40);
  m_buf.push_back((val & 0x000000ff00000000) >> 32);
  m_buf.push_back((val & 0x00000000ff000000) >> 24);
  m_buf.push_back((val & 0x0000000000ff0000) >> 16);
  m_buf.push_back((val & 0x000000000000ff00) >> 8);
  m_buf.push_back((val & 0x00000000000000ff) >> 0);
}

void msgpack_encoder::_buf_out_str(const char *s, size_t len)
{
  if (len <= 31) {
    m_buf.push_back(0xa0 | len);
  }
  else if (len <= UINT8_MAX) {
    m_buf.push_back(HEX_STR8);
    _push_be((uint8_t)len);
  }
  else if (len <= UINT16_MAX) {
    m_buf.push_back(HEX_STR16);
    _push_be((uint16_t)len);
  }
  else {
    m_buf.push_back(HEX_STR32);
    _push_be((uint32_t)len);
  }
  const char *end = s + len;
  for (const char *p = s; p < end; p++) {
    m_buf.push_back(*p);
  }
}


void msgpack_encoder::_buf_out(const node &n) {
  switch (n.get_type()) {
    case type::NIL:
      m_buf.push_back(HEX_NIL);
      return;

    case type::BOOL:
      if (n == true) {
        m_buf.push_back(HEX_TRUE);
      }
      else {
        m_buf.push_back(HEX_FALSE);
      }
      return;

    case type::INT64:
      {
        int64_t val = n.as_int64();
        if (val < 0) {
          if (val >= -32) {
            m_buf.push_back(val);
          }
          else if (val >= INT8_MIN) {
            m_buf.push_back(HEX_INT8);
            _push_be(union_cast<int64_t, uint8_t>(val));
          }
          else if (val >= INT16_MIN) {
            m_buf.push_back(HEX_INT16);
            _push_be(union_cast<int64_t, uint16_t>(val));
          }
          else if (val >= INT32_MIN) {
            m_buf.push_back(HEX_INT32);
            _push_be(union_cast<int64_t, uint32_t>(val));
          }
          else {
            m_buf.push_back(HEX_INT64);
            _push_be(union_cast<int64_t, uint64_t>(val));
          }
        }
        else {
          if (val <= 127) {
            m_buf.push_back(val);
          }
          else if (val <= UINT8_MAX) {
            m_buf.push_back(HEX_UINT8);
            _push_be(union_cast<int64_t, uint8_t>(val));
          }
          else if (val <= UINT16_MAX) {
            m_buf.push_back(HEX_UINT16);
            _push_be(union_cast<int64_t, uint16_t>(val));
          }
          else if (val <= UINT32_MAX) {
            m_buf.push_back(HEX_UINT32);
            _push_be(union_cast<int64_t, uint32_t>(val));
          }
          else {
            /* Ellis doesn't support uint64 */
            ELLIS_ASSERT_UNREACHABLE();
          }
        }
      }
      return;

    case type::DOUBLE:
      {
        double d = n.as_double();
        m_buf.push_back(HEX_FLOAT64);
        uint64_t val = union_cast<double, uint64_t>(d);
        _push_be(val);
      }
      return;

    case type::U8STR:
      {
        const u8str_node &s = n.as_u8str();
        _buf_out_str(s.c_str(), s.length());
      }
      return;

    case type::ARRAY:
      {
        const array_node &a = n.as_array();
        size_t len = a.length();
        if (len <= 15) {
          m_buf.push_back(0x90 | len);
        }
        else if (len <= UINT16_MAX) {
          m_buf.push_back(HEX_ARRAY16);
          _push_be((uint16_t)len);
        }
        else {
          m_buf.push_back(HEX_ARRAY32);
          _push_be((uint32_t)len);
        }
        a.foreach([this](const node &item) {
          _buf_out(item);
        });
      }
      return;

    case type::BINARY:
      {
        const binary_node &b = n.as_binary();
        size_t len = b.length();
        if (len <= UINT8_MAX) {
          m_buf.push_back(HEX_BIN8);
          _push_be((uint8_t)len);
        }
        else if (len <= UINT16_MAX) {
          m_buf.push_back(HEX_BIN16);
          _push_be((uint16_t)len);
        }
        else {
          m_buf.push_back(HEX_BIN32);
          _push_be((uint32_t)len);
        }
        const byte *end = b.data() + len;
        for (const byte *p = b.data(); p < end; p++) {
          m_buf.push_back(*p);
        }
      }
      return;

    case type::MAP:
      {
        const map_node &m = n.as_map();
        size_t len = m.length();
        if (len <= 15) {
          m_buf.push_back(0x80 | len);
        }
        else if (len <= UINT16_MAX) {
          m_buf.push_back(HEX_MAP16);
          _push_be((uint16_t)len);
        }
        else if (len <= UINT32_MAX) {
          m_buf.push_back(HEX_MAP32);
          _push_be((uint32_t)len);
        }
        else {
          THROW_ELLIS_ERR(TRANSLATE_FAIL, "Too many map entries for msgpack");
        }
        m.foreach([this](const string &k, const node &v) {
          _buf_out_str(k.c_str(), k.length());
          _buf_out(v);
        });
      }
      return;
  }
}


progress msgpack_encoder::fill_buffer(
    byte *buf,
    size_t *bytecount)
{
  size_t avail = m_end - m_pos;
  size_t actual_bc = std::min(*bytecount, avail);
  std::memcpy(buf, m_buf.data() + m_pos, actual_bc);
  m_pos += actual_bc;
  *bytecount -= actual_bc;
  if (m_pos == m_end) {
    return progress(true);
  }
  return progress(stream_state::CONTINUE);
}


void msgpack_encoder::reset(const node *new_node)
{
  m_buf.clear();
  _buf_out(*new_node);
  m_pos = 0;
  m_end = m_buf.size();
}


}  /* namespace ellis */
