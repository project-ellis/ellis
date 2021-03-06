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


#include <ellis/core/node.hpp>

#include <ellis/core/array_node.hpp>
#include <ellis/core/binary_node.hpp>
#include <ellis/core/err.hpp>
#include <ellis/core/map_node.hpp>
#include <ellis/core/system.hpp>
#include <ellis/core/type.hpp>
#include <ellis/core/u8str_node.hpp>
#include <ellis_private/core/payload.hpp>
#include <ellis_private/using.hpp>
#include <stddef.h>
#include <string.h>

namespace ellis {


/** A convenience macro for preparing for a non-const operation. */
#define MIGHTALTER() _prep_for_write()


/** A convenience macro for throwing an error when type is improper. */
#define VERIFY_TYPE(typ) \
  do { \
    if (get_type() != type::typ) { \
      THROW_ELLIS_ERR(TYPE_MISMATCH, "not " #typ); \
    } \
  } while (0)


#define OPFUNCS_EQ_PRIMITIVE(typ, e_typ, short_typ) \
bool node::operator==(typ o) const \
{ \
  if (type(m_type) != type::e_typ) { \
    return false; \
  } \
  return m_##short_typ == o; \
} \
\
bool node::operator!=(typ o) const \
{ \
  return not (*this == o); \
}


#define OPFUNC_ASSIGN_PRIMITIVE(typ, short_typ, e_typ) \
node& node::operator=(typ o) \
{ \
  _release_contents(); \
  m_type = (int)type::e_typ; \
  m_##short_typ = o; \
  return *this; \
}


#define ASFUNCS_PRIMITIVE(typ, f_typ, e_typ, short_typ) \
\
typ & node::_as_mutable_##f_typ() \
{ \
  MIGHTALTER(); \
  return m_##short_typ; \
} \
typ & node::as_mutable_##f_typ() \
{ \
  VERIFY_TYPE(e_typ); \
  MIGHTALTER(); \
  return m_##short_typ; \
} \
\
const typ & node::_as_##f_typ() const \
{ \
  return m_##short_typ; \
} \
\
const typ & node::as_##f_typ() const \
{ \
  VERIFY_TYPE(e_typ); \
  return m_##short_typ; \
}
/* End of ASFUNCS_PRIMITIVE macro. */


#define _OPFUNC_NODE_CMP(op, verb) \
bool operator op(const node &a, const node &b) \
{ \
  type a_type = a.get_type(); \
  type b_type = b.get_type(); \
\
  if (a_type == type::INT64 && b_type == type::INT64) { \
    return a.as_int64() op b.as_int64(); \
  } \
  else if (a_type == type::DOUBLE && b_type == type::DOUBLE) { \
    return a.as_double() op b.as_double(); \
  } \
  else { \
    THROW_ELLIS_ERR( TYPE_MISMATCH, \
        "types " << type_str(a_type) << " and " << type_str(b_type) \
        << "can't be " #verb); \
  } \
}


#define OPFUNC_NODE_CMP \
  _OPFUNC_NODE_CMP(<, compared) \
  _OPFUNC_NODE_CMP(<=, compared) \
  _OPFUNC_NODE_CMP(>, compared) \
  _OPFUNC_NODE_CMP(>=, compared)
/* End of ASFUNCS_NODE_CMP macro. */


#define _OPFUNC_NODE_ARITHMETIC(op, verb) \
node operator op(const node &a, const node &b) \
{ \
  type a_type = a.get_type(); \
  type b_type = b.get_type(); \
\
  if (a_type == type::INT64 && b_type == type::INT64) { \
    return node(a.as_int64() op b.as_int64()); \
  } \
  else if (a_type == type::DOUBLE && b_type == type::DOUBLE) { \
    return node(a.as_double() op b.as_double()); \
  } \
  else { \
    THROW_ELLIS_ERR( TYPE_MISMATCH, \
        "types " << type_str(a_type) << " and " << type_str(b_type) \
        << "can't be " #verb); \
  } \
} \
node node::operator op##=(const node &o) \
{ \
  type this_type = get_type(); \
  type o_type = o.get_type(); \
  if (this_type == type::INT64 && o_type == type::INT64) { \
    as_mutable_int64() += o.as_int64(); \
  } \
  else if (this_type == type::DOUBLE && o_type == type::DOUBLE) { \
    as_mutable_double() += o.as_double(); \
  } \
  else { \
    THROW_ELLIS_ERR( TYPE_MISMATCH, \
        "types " << type_str(this_type) << " and " << type_str(o_type) \
        << "can't be " #verb); \
  } \
  return *this; \
}


#define OPFUNC_NODE_ARITHMETIC \
  _OPFUNC_NODE_ARITHMETIC(+, added) \
  _OPFUNC_NODE_ARITHMETIC(-, subtracted) \
  _OPFUNC_NODE_ARITHMETIC(*, multiplied) \
  _OPFUNC_NODE_ARITHMETIC(/, divided) \
/* End of ASFUNCS_NODE_OPS macro. */


#define _OPFUNC_CMP(op, typ, f_typ) \
bool operator op(const node &a, typ b) \
{ \
  return a.as_##f_typ() op b; \
} \
bool operator op(typ a, const node &b) \
{ \
  return a op b.as_##f_typ(); \
 \
}


#define OPFUNCS_CMP(typ, f_typ) \
  _OPFUNC_CMP(<, typ, f_typ) \
  _OPFUNC_CMP(<=, typ, f_typ) \
  _OPFUNC_CMP(>, typ, f_typ) \
  _OPFUNC_CMP(>=, typ, f_typ)
/* End of ASFUNCS_CMP macro. */


#define _OPFUNC_ARITHMETIC(op, typ, f_typ, e_typ, short_typ) \
node operator op(const node &a, typ b) \
{ \
  return node(a.as_##f_typ() op b); \
} \
typ operator op(typ a, const node &b) \
{ \
  return a op b.as_##f_typ(); \
 \
} \
node node::operator op##=(typ o) \
{ \
  VERIFY_TYPE(e_typ); \
  m_##short_typ op##= o; \
  return *this; \
}


#define OPFUNCS_ARITHMETIC(typ, f_typ, e_typ, short_typ) \
  _OPFUNC_ARITHMETIC(+, typ, f_typ, e_typ, short_typ) \
  _OPFUNC_ARITHMETIC(-, typ, f_typ, e_typ, short_typ) \
  _OPFUNC_ARITHMETIC(*, typ, f_typ, e_typ, short_typ) \
  _OPFUNC_ARITHMETIC(/, typ, f_typ, e_typ, short_typ)
/* End of ASFUNCS_ARITHMETIC macro. */


#define ASFUNCS_CONTAINER(typ, ret_typ, e_typ) \
ret_typ & node::_as_mutable_##typ() \
{ \
  MIGHTALTER(); \
  return *(reinterpret_cast<ret_typ*>(this)); \
} \
\
ret_typ & node::as_mutable_##typ() \
{ \
  VERIFY_TYPE(e_typ); \
  MIGHTALTER(); \
  return *(reinterpret_cast<ret_typ*>(this)); \
} \
\
const ret_typ & node::_as_##typ() const \
{ \
  return *(reinterpret_cast<const ret_typ*>(this)); \
} \
\
const ret_typ & node::as_##typ() const \
{ \
  VERIFY_TYPE(e_typ); \
  return *(reinterpret_cast<const ret_typ*>(this)); \
} \
/* End of ASFUNCS_CONTAINER macro. */


/** Convenience macro for typecast operators on primitive types.
 */
#define OPFUNC_CAST_PRIMITIVE(typ, f_typ) \
node::operator typ() const \
{ \
  return as_##f_typ(); \
}


static_assert((int)type::BOOL   <  (int)type::refcntd, "refcount check broken");
static_assert((int)type::NIL    <  (int)type::refcntd, "refcount check broken");
static_assert((int)type::BOOL   <  (int)type::refcntd, "refcount check broken");
static_assert((int)type::INT64  <  (int)type::refcntd, "refcount check broken");
static_assert((int)type::DOUBLE <  (int)type::refcntd, "refcount check broken");
static_assert((int)type::U8STR  >= (int)type::refcntd, "refcount check broken");
static_assert((int)type::ARRAY  >= (int)type::refcntd, "refcount check broken");
static_assert((int)type::BINARY >= (int)type::refcntd, "refcount check broken");
static_assert((int)type::MAP    >= (int)type::refcntd, "refcount check broken");


/** Does this type enum value represent a type for which refcount is used? */
static inline bool _is_refcounted(int t)
{
  return t >= (int)type::refcntd;
}


/** Initialize contents to pristine state, assuming no prior contents.
 *
 * If m_type indicates a pointer in the union, it will be set up;
 * or if an in-place constructor is needed, it will be called.
 */
void node::_zap_contents(type t)
{
  using namespace ::ellis::payload_types;

  m_type = (int)t;
  if (_is_refcounted(m_type)) {
    /*
     * Use malloc/free here to make sure we don't accidentally call a union
     * constructor or similar via new/delete.
     */
    // TODO: this is wasteful because it always allocates the max memory that
    // any payload might take.
    m_pay = (payload*)malloc(sizeof(*m_pay));
    m_pay->m_refcount = 1;
    switch (type(m_type)) {
      case type::ARRAY:
        new (&(m_pay->m_arr)) arr_t();
        break;

      case type::BINARY:
        new (&(m_pay->m_bin)) bin_t();
        break;

      case type::MAP:
        new (&(m_pay->m_map)) map_t();
        break;

      case type::U8STR:
        new (&(m_pay->m_str)) str_t();
        break;

      default:
        /* Never hit, due to _is_refcounted. */
        ELLIS_ASSERT_UNREACHABLE();
        break;
    }
  }
  else {
    m_pay = nullptr;
  }
}


/** Grab state from other node, assuming no prior state.
 *
 * If the node already has contents, caller should call _release_contents()
 * before calling this function.
 */
void node::_grab_contents(const node& other)
{
  m_type = other.m_type;
  if (_is_refcounted(m_type)) {
    m_pay = other.m_pay;
    m_pay->m_refcount++;
  }
  else {
    memcpy(m_pad, other.m_pad,  // NOLINT
        offsetof(node, m_type) - offsetof(node, m_pad));
  }
}


/** Release the contents.
 *
 * It is safe to call _release_contents multiple times, which has the same
 * effect as calling it once.
 */
void node::_release_contents()
{
  using namespace ::ellis::payload_types;
  if (_is_refcounted(m_type)) {
    m_pay->m_refcount--;
    if (m_pay->m_refcount == 0) {
      switch (type(m_type)) {
        case type::ARRAY:
          m_pay->m_arr.~arr_t();
          break;

        case type::BINARY:
          m_pay->m_bin.~bin_t();
          break;

        case type::MAP:
          m_pay->m_map.~map_t();
          break;

        case type::U8STR:
          m_pay->m_str.~str_t();
          break;

        default:
          /* Never hit, due to _is_refcounted. */
          ELLIS_ASSERT_UNREACHABLE();
          break;
      }
      free(m_pay);
      m_pay = nullptr;
    }
  }
  /*
   * Set to NIL so that we don't do a string destruction or decref twice.
   */
  m_type = (int)type::NIL;
}


/** Make sure that it is safe to write (that is, copy if necessary in
 * support of COW (copy on write) behavior.
 */
void node::_prep_for_write()
{
  if (!_is_refcounted(m_type)) {
    /* Nothing to do for a primitive type. */
    return;
  }
  ELLIS_ASSERT_GT(m_pay->m_refcount, 0);
  if (m_pay->m_refcount == 1) {
    /* Nothing to do, this is the only copy, so go ahead and write. */
    return;
  }
  /* This is a shared node.  Copy before writing. */
  deep_copy(*this);
}


OPFUNCS_EQ_PRIMITIVE(bool, BOOL, boo)
OPFUNCS_EQ_PRIMITIVE(double, DOUBLE, dbl)
OPFUNCS_EQ_PRIMITIVE(int, INT64, int)
OPFUNCS_EQ_PRIMITIVE(unsigned int, INT64, int)
OPFUNCS_EQ_PRIMITIVE(int64_t, INT64, int)


OPFUNC_ASSIGN_PRIMITIVE(bool, boo, BOOL)
OPFUNC_ASSIGN_PRIMITIVE(double, dbl, DOUBLE)
OPFUNC_ASSIGN_PRIMITIVE(int, int, INT64)
OPFUNC_ASSIGN_PRIMITIVE(unsigned int, int, INT64)
OPFUNC_ASSIGN_PRIMITIVE(int64_t, int, INT64)

ASFUNCS_PRIMITIVE(bool, bool, BOOL, boo)
ASFUNCS_PRIMITIVE(double, double, DOUBLE, dbl)
ASFUNCS_PRIMITIVE(int64_t, int64, INT64, int)

ASFUNCS_CONTAINER(array, array_node, ARRAY)
ASFUNCS_CONTAINER(map, map_node, MAP)
ASFUNCS_CONTAINER(binary, binary_node, BINARY)
ASFUNCS_CONTAINER(u8str, u8str_node, U8STR)


OPFUNC_CAST_PRIMITIVE(bool, bool)
OPFUNC_CAST_PRIMITIVE(double, double)
OPFUNC_CAST_PRIMITIVE(int, int64)
OPFUNC_CAST_PRIMITIVE(unsigned int, int64)
OPFUNC_CAST_PRIMITIVE(int64_t, int64)

node::operator const char *() const
{
  return as_u8str().c_str();
}


node::node(type t)
{
  _zap_contents(t);
}


node::node(const byte *mem, size_t bytes)
{
  _zap_contents(type::BINARY);
  m_pay->m_bin.resize(bytes);
  memcpy(m_pay->m_bin.data(), mem, bytes);
}


node::node(bool b)
{
  _zap_contents(type::BOOL);
  m_boo = b;
}


node::node(int i)
{
  _zap_contents(type::INT64);
  m_int = i;
}


node::node(int64_t i)
{
  _zap_contents(type::INT64);
  m_int = i;
}


node::node(unsigned int i)
{
  _zap_contents(type::INT64);
  m_int = i;
}


node::node(double d)
{
  _zap_contents(type::DOUBLE);
  m_dbl = d;
}


node::node(const std::string& s)
{
  _zap_contents(type::U8STR);
  _as_mutable_u8str().assign(s.c_str());
}


node::node(const char *s)
{
  _zap_contents(type::U8STR);
  _as_mutable_u8str().assign(s);
}


node::node(const initializer_list<node> &elems)
{
  _zap_contents(type::ARRAY);
  auto & a = _as_mutable_array();
  for (const auto &e : elems) {
    a.append(e);
  }
}


node::node(const initializer_list<pair<const char *, node>> &kvpairs)
{
  _zap_contents(type::MAP);
  auto & m = _as_mutable_map();
  for (const auto &kv : kvpairs) {
    m.insert(kv.first, kv.second);
  }
}


node::node(const node& other)
{
  _grab_contents(other);
}


node::node(node&& other)
{
  _grab_contents(other);
  other._release_contents();
}


node::~node()
{
  _release_contents();
}


void node::swap(node &other)
{
  node tmp(*this);
  *this = other;
  other = tmp;
}


void node::deep_copy(const node &o)
{
  /* Make a tmp copy to preserve contents in case &o == this. */
  node tmp(o);

  /* Release, zap, and copy from tmp. */
  _release_contents();
  _zap_contents(tmp.get_type());
  if (_is_refcounted(m_type)) {
    switch (type(m_type)) {
      case type::ARRAY:
        m_pay->m_arr = tmp.m_pay->m_arr;
        break;

      case type::BINARY:
        m_pay->m_bin = tmp.m_pay->m_bin;
        break;

      case type::MAP:
        m_pay->m_map = tmp.m_pay->m_map;
        break;

      case type::U8STR:
        m_pay->m_str = tmp.m_pay->m_str;
        break;

      default:
        /* Never hit, due to _is_refcounted. */
        ELLIS_ASSERT_UNREACHABLE();
        break;
    }
  }
  else {
    memcpy(m_pad, tmp.m_pad,  // NOLINT
        offsetof(node, m_type) - offsetof(node, m_pad));
  }
}


node& node::operator=(const node& rhs)
{
  _release_contents();
  _grab_contents(rhs);
  return *this;
}


node& node::operator=(node&& rhs)
{
  _release_contents();
  _grab_contents(rhs);
  rhs._release_contents();
  return *this;
}


node& node::operator=(const char *s)
{
  _release_contents();
  _zap_contents(type::U8STR);
  _as_mutable_u8str().assign(s);
  return *this;
}


node& node::operator=(const std::string &s)
{
  _release_contents();
  _zap_contents(type::U8STR);
  _as_mutable_u8str().assign(s.c_str());
  return *this;
}


bool node::operator==(const node &o) const
{
  if (m_type != o.m_type) {
    return false;
  }
  switch (type(m_type)) {
    case type::BOOL:
      return m_boo == o.m_boo;

    case type::DOUBLE:
      return m_dbl == o.m_dbl;

    case type::INT64:
      return m_int == o.m_int;

    case type::NIL:
      return true;  /* Both nil, nothing more to say. */

    case type::ARRAY:
      return _as_array() == o._as_array();

    case type::BINARY:
      return _as_binary() == o._as_binary();

    case type::MAP:
      return _as_map() == o._as_map();

    case type::U8STR:
      return _as_u8str() == o._as_u8str();

  }
  /* Never reached. */
  ELLIS_ASSERT_UNREACHABLE();
  return true;
}


bool node::operator!=(const node &o) const
{
  return not (*this == o);
}


bool node::operator==(const char *s) const
{
  if (type(m_type) != type::U8STR) {
    return false;
  }
  return strcmp(_as_u8str().c_str(), s) == 0;
}


bool node::operator!=(const char *s) const
{
  return not (*this == s);
}


bool node::operator==(const string &s) const
{
  if (type(m_type) != type::U8STR) {
    return false;
  }
  return s == _as_u8str().c_str();
}


bool node::operator!=(const string &s) const
{
  return not (*this == s);
}


bool node::is_type(type t) const
{
  return get_type() == t;
}


type node::get_type() const
{
  return (type)m_type;
}


using map_selector_cb = std::function<void(
    const string & pattern,
    size_t path_position)>;


using array_selector_cb = std::function<void(
    size_t start,
    size_t stop,
    size_t path_position)>;


static void parse_path(
    const string & path,
    const map_selector_cb & got_map_selector,
    const array_selector_cb & got_array_selector)
{
  enum class parse_state {
    NEED_SELECTOR,
    NEED_INDEX,
    IN_INDEX,
    IN_KEY,
  };

#define BOOM(DETAILS) \
  do { \
    THROW_ELLIS_ERR(PATH_FAIL, \
      "path parsing failure at position " << (curr - path_start) \
      << " of path " << path << ": " << DETAILS); \
  } while (0)

  parse_state state = parse_state::NEED_SELECTOR;
  const char *path_start = path.c_str();
  const char *curr = path_start;
  const char *key_start = nullptr;  /* init val irrelevant due to state graph */
  size_t index = 0;  /* init val irrelevant due to state graph. */

  for (; *curr; curr++) {
    char c = *curr;

    switch (state) {

      case parse_state::NEED_SELECTOR:
        if (isspace(c)) {
          /* Do nothing: ignore spaces between selectors. */
        }
        else if (c == '{') {
          key_start = curr + 1;
          state = parse_state::IN_KEY;
        }
        else if (c == '[') {
          state = parse_state::NEED_INDEX;
        }
        else {
          BOOM("need [ or . selector");
        }
        break;

      case parse_state::NEED_INDEX:
        if (isdigit(c)) {
          index = c - '0';
          state = parse_state::IN_INDEX;
        }
        else {
          BOOM("need a number for index");
        }
        break;

      case parse_state::IN_INDEX:
        if (isdigit(c)) {
          index = index * 10 + c - '0';
        }
        else if (c == ']') {
          got_array_selector(index, index, curr - path_start);
          state = parse_state::NEED_SELECTOR;
        }
        else {
          BOOM("not a digit");
        }
        break;

      case parse_state::IN_KEY:
        if (c == '}') {
          string pattern(key_start, curr - key_start);
          got_map_selector(pattern, curr - path_start);
          state = parse_state::NEED_SELECTOR;
        }
        else { /* A character in the pattern. */
          /* Do nothing: we have a pointer to start of pattern; wait for end. */
        }
        break;

    }  // switch(state)

  }  // for

  if (state != parse_state::NEED_SELECTOR) {
    BOOM("unexpected path termination");
  }

#undef BOOM
  return;
}


node & node::at_mutable(const std::string &path)
{
  MIGHTALTER();
  /* Re-use code from const version. */
  return const_cast<node&>(
      static_cast<const node*>(this)->at(path));
}


const node & node::at(const std::string &path) const
{
  /* Prepare for parsing/traversing path by setting up state and callbacks. */
  struct mystate_t {
    const node *v;
  };

  mystate_t mystate;
  mystate.v = this;

#define BOOM(POS, DETAILS) \
  do { \
    THROW_ELLIS_ERR(PATH_FAIL, \
      "map access failure at position " << (POS) \
      << " of path " << path << ": " << DETAILS); \
  } while (0)

  auto got_map_selector =
    [&mystate, &path]
    (const string &pattern, size_t pos)
    {
      if (! mystate.v->is_type(type::MAP)) {
        BOOM(pos, "map pattern selector applied to non-map");
      }
      if (! mystate.v->_as_map().has_key(pattern)) {
        BOOM(pos, "pattern not found in map");
      }
      mystate.v = &(mystate.v->_as_map()[pattern]);
    };
#undef BOOM

#define BOOM(DETAILS, POS) \
  do { \
    THROW_ELLIS_ERR(PATH_FAIL, \
      "array access failure at position " << (POS) \
      << " of path " << path << ": " << DETAILS); \
  } while (0)

  auto got_array_selector =
    [&mystate, &path]
    (size_t start, size_t stop, size_t pos)
    {
      if (start != stop) {
        BOOM(pos, "array range not supported in this mode");
      }
      if (! mystate.v->is_type(type::ARRAY)) {
        BOOM(pos, "array index applied to non-array");
      }
      if (start >= mystate.v->_as_array().length()) {
        BOOM(pos, "index out of range");
      }
      mystate.v = &(mystate.v->_as_array()[start]);
    };
#undef BOOM

  /* Invoke path parsing/traversing logic. */
  parse_path(path, got_map_selector, got_array_selector);

  /* If haven't thrown exception by now, should have the desired node. */
  return *(mystate.v);
}


node & node::install(const std::string &path, const node &newval)
{
  /* Prepare for parsing/traversing path by setting up state and callbacks. */
  struct mystate_t {
    node *v;
  };

  mystate_t mystate;
  mystate.v = this;

#define BOOM(POS, DETAILS) \
  do { \
    THROW_ELLIS_ERR(PATH_FAIL, \
      "map access failure at position " << (POS) \
      << " of path " << path << ": " << DETAILS); \
  } while (0)

  auto got_map_selector =
    [&mystate, &path]
    (const string &pattern, size_t pos)
    {
      if (! mystate.v->is_type(type::MAP)
          && ! mystate.v->is_type(type::NIL)) {
        BOOM(pos, "map pattern selector applied to non-map");
      }
      if (mystate.v->is_type(type::NIL)) {
        *(mystate.v) = node(type::MAP);
      }
      if (! mystate.v->_as_map().has_key(pattern)) {
        mystate.v->_as_mutable_map().insert(pattern, node(type::NIL));
      }
      mystate.v = &(mystate.v->_as_mutable_map()[pattern]);
    };
#undef BOOM

#define BOOM(DETAILS, POS) \
  do { \
    THROW_ELLIS_ERR(PATH_FAIL, \
      "array access failure at position " << (POS) \
      << " of path " << path << ": " << DETAILS); \
  } while (0)

  auto got_array_selector =
    [&mystate, &path]
    (size_t start, size_t stop, size_t pos)
    {
      if (start != stop) {
        BOOM(pos, "array range not supported in this mode");
      }
      if (! mystate.v->is_type(type::ARRAY)
          && ! mystate.v->is_type(type::NIL)) {
        BOOM(pos, "array index applied to non-array");
      }
      if (mystate.v->is_type(type::NIL)) {
        *(mystate.v) = node(type::ARRAY);
      }
      while (start >= mystate.v->_as_array().length()) {
        mystate.v->_as_mutable_array().append(node(type::NIL));
      }
      mystate.v = &(mystate.v->_as_mutable_array()[start]);
    };
#undef BOOM

  /* Invoke path parsing/traversing logic. */
  parse_path(path, got_map_selector, got_array_selector);

  /* If haven't thrown exception by now, we should be in the right place to
   * install the new node. */
  *(mystate.v) = newval;
  return *(mystate.v);
}


std::ostream & operator<<(std::ostream & os, const node & v)
{
  switch (type(v.get_type())) {
    case type::NIL:
      return os << "NIL";

    case type::BOOL:
      return os << v.as_bool();

    case type::INT64:
      return os << v.as_int64();

    case type::DOUBLE:
      return os << v.as_double();

    case type::U8STR:
      return os << v.as_u8str();

    case type::ARRAY:
      return os << v.as_array();

    case type::BINARY:
      return os << v.as_binary();

    case type::MAP:
      return os << v.as_map();
  }
  ELLIS_ASSERT_UNREACHABLE();
}


OPFUNC_NODE_CMP

OPFUNC_NODE_ARITHMETIC

OPFUNCS_CMP(int, int64)
OPFUNCS_CMP(unsigned int, int64)
OPFUNCS_CMP(int64_t, int64)
OPFUNCS_CMP(double, double)

OPFUNCS_ARITHMETIC(int, int64, INT64, int)
OPFUNCS_ARITHMETIC(unsigned int, int64, INT64, int)
OPFUNCS_ARITHMETIC(int64_t, int64, INT64, int)

OPFUNCS_ARITHMETIC(double, double, DOUBLE, dbl)


}  /* namespace ellis */
