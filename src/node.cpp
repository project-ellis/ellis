#include <ellis/node.hpp>

#include <ellis/array_node.hpp>
#include <ellis/binary_node.hpp>
#include <ellis/err.hpp>
#include <ellis/map_node.hpp>
#include <ellis/private/using.hpp>
#include <sstream>
#include <stddef.h>
#include <string.h>

namespace ellis {


/** A convenience macro for preparing for a non-const operation. */
#define MIGHTALTER() _prep_for_write()


/** A convenience macro for throwing an error when type is improper. */
#define TYPE_VERIFY(typ) \
  do { \
    if (get_type() != type::typ) { \
      throw MAKE_ELLIS_ERR(err_code::WRONG_TYPE, "not " #typ); \
    } \
  } while (0)


/** The lowest numerical enum value of any type for which refcount is used. */
static const int k_lowest_rc = 5;

static_assert((int)type::BOOL   <  k_lowest_rc, "refcount check broken");
static_assert((int)type::NIL    <  k_lowest_rc, "refcount check broken");
static_assert((int)type::BOOL   <  k_lowest_rc, "refcount check broken");
static_assert((int)type::INT64  <  k_lowest_rc, "refcount check broken");
static_assert((int)type::DOUBLE <  k_lowest_rc, "refcount check broken");
static_assert((int)type::U8STR  <  k_lowest_rc, "refcount check broken");
static_assert((int)type::ARRAY  >= k_lowest_rc, "refcount check broken");
static_assert((int)type::BINARY >= k_lowest_rc, "refcount check broken");
static_assert((int)type::MAP    >= k_lowest_rc, "refcount check broken");


/** Does this type enum value represent a type for which refcount is used? */
static inline bool _is_refcounted(int t)
{
  return t >= k_lowest_rc;
}


/** Initialize contents to pristine state, assuming no prior contents.
 *
 * If m_type indicates a pointer in the union, it will be set up;
 * or if an in-place constructor is needed, it will be called.
 */
void node::_zap_contents(type t)
{
  using namespace ::ellis::prize_types;

  m_type = (int)t;
  if (type(m_type) == type::U8STR) {
    new (&m_str) std::string();
  }
  else if (_is_refcounted(m_type)) {
    /*
     * Use malloc/free here to make sure we don't accidentally call a union
     * constructor or similar via new/delete.
     */
    m_blk = (prize_blk*)malloc(sizeof(*m_blk));
    m_blk->m_refcount = 1;
    switch (type(m_type)) {
      case type::ARRAY:
        new (&(m_blk->m_arr)) arr_t();
        break;

      case type::BINARY:
        new (&(m_blk->m_bin)) bin_t();
        break;

      case type::MAP:
        new (&(m_blk->m_map)) map_t();
        break;

      default:
        /* Never hit, due to _is_refcounted. */
        assert(0);
        break;
    }
  }
  else {
    m_blk = nullptr;
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
  if (type(m_type) == type::U8STR) {
    /* In-place string copy constructor. */
    new (&m_str) std::string(other.m_str);
  }
  else if (_is_refcounted(m_type)) {
    m_blk = other.m_blk;
    m_blk->m_refcount++;
  }
  else {
    static_assert(
        sizeof(m_pad) == offsetof(node, m_type),
        "Uh oh, m_pad does not seem to cover entire union...");
    memcpy(m_pad, other.m_pad, sizeof(m_pad));
  }
}


/** Release the contents.
 *
 * It is safe to call _release_contents multiple times, which has the same
 * effect as calling it once.
 */
void node::_release_contents()
{
  using namespace ::ellis::prize_types;
  if (type(m_type) == type::U8STR) {
    m_str.~basic_string<char>();
  }
  else if (_is_refcounted(m_type)) {
    m_blk->m_refcount--;
    if (m_blk->m_refcount == 0) {
      switch (type(m_type)) {
        case type::ARRAY:
          m_blk->m_arr.~arr_t();
          break;

        case type::BINARY:
          m_blk->m_bin.~bin_t();
          break;

        case type::MAP:
          m_blk->m_map.~map_t();
          break;

        default:
          /* Never hit, due to _is_refcounted. */
          assert(0);
          break;
      }
      free(m_blk);
      m_blk = nullptr;
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
  assert(m_blk->m_refcount > 0);
  if (m_blk->m_refcount == 1) {
    /* Nothing to do, this is the only copy, so go ahead and write. */
    return;
  }
  /* This is a shared node.  Copy before writing. */
  deep_copy(*this);
}


// TODO: macro it


/** Unchecked version of similarly named public function. */
bool & node::_as_bool()
{
  return m_boo;
}


/** Unchecked version of similarly named public function. */
const bool & node::_as_bool() const
{
  return m_boo;
}


/** Unchecked version of similarly named public function. */
int64_t & node::_as_int64()
{
  return m_int;
}


/** Unchecked version of similarly named public function. */
const int64_t & node::_as_int64() const
{
  return m_int;
}


/** Unchecked version of similarly named public function. */
double & node::_as_double()
{
  return m_dbl;
}


/** Unchecked version of similarly named public function. */
const double & node::_as_double() const
{
  return m_dbl;
}


/** Unchecked version of similarly named public function. */
const std::string & node::_as_u8str() const
{
  return m_str;
}


/** Unchecked version of similarly named public function. */
array_node & node::_as_array()
{
  MIGHTALTER();
  /* Re-use code from const version. */
  return const_cast<array_node&>(
      static_cast<const node*>(this)->_as_array());
}


/** Unchecked version of similarly named public function. */
const array_node & node::_as_array() const
{
  return *(reinterpret_cast<const array_node*>(this));
}


/** Unchecked version of similarly named public function. */
map_node & node::_as_map()
{
  MIGHTALTER();
  /* Re-use code from const version. */
  return const_cast<map_node&>(
      static_cast<const node*>(this)->_as_map());
}


/** Unchecked version of similarly named public function. */
const map_node & node::_as_map() const
{
  return *(reinterpret_cast<const map_node*>(this));
}


/** Unchecked version of similarly named public function. */
binary_node & node::_as_binary()
{
  MIGHTALTER();
  /* Re-use code from const version. */
  return const_cast<binary_node&>(
      static_cast<const node*>(this)->_as_binary());
}


/** Unchecked version of similarly named public function. */
const binary_node & node::_as_binary() const
{
  return *(reinterpret_cast<const binary_node*>(this));
}


node::node(type t)
{
  _zap_contents(t);
}


node::node(const uint8_t *mem, size_t bytes)
{
  _zap_contents(type::BINARY);
  m_blk->m_bin.resize(bytes);
  memcpy(m_blk->m_bin.data(), mem, bytes);
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


node::node(uint64_t i)
{
  if (i > INT64_MAX) {
    throw std::out_of_range("64 bit unsigned integer too large");
  }
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
  m_str = s;
}


node::node(const char *s)
{
  _zap_contents(type::U8STR);
  m_str = s;
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
        m_blk->m_arr = tmp.m_blk->m_arr;
        break;

      case type::BINARY:
        m_blk->m_bin = tmp.m_blk->m_bin;
        break;

      case type::MAP:
        m_blk->m_map = tmp.m_blk->m_map;
        break;

      default:
        /* Never hit, due to _is_refcounted. */
        assert(0);
        break;
    }
  }
  else {
    memcpy(m_pad, tmp.m_pad, sizeof(pad_t));
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

    case type::U8STR:
      return m_str == o.m_str;

    case type::ARRAY:
      return _as_array() == o._as_array();

    case type::BINARY:
      return _as_binary() == o._as_binary();

    case type::MAP:
      return _as_map() == o._as_map();
  }
  /* Never reached. */
  assert(0);
  return true;
}


bool node::operator!=(const node &o) const
{
  return not (*this == o);
}


bool node::is_type(type t) const
{
  return get_type() == t;
}


type node::get_type() const
{
  return (type)m_type;
}


node::operator int64_t() const
{
  return as_int64();
}


node::operator double() const
{
  return as_double();
}


bool & node::as_bool()
{
  TYPE_VERIFY(BOOL);
  return _as_bool();
}


const bool & node::as_bool() const
{
  TYPE_VERIFY(BOOL);
  return _as_bool();
}


int64_t & node::as_int64()
{
  TYPE_VERIFY(INT64);
  return _as_int64();
}


const int64_t & node::as_int64() const
{
  TYPE_VERIFY(INT64);
  return _as_int64();
}


double & node::as_double()
{
  TYPE_VERIFY(DOUBLE);
  return _as_double();
}


const double & node::as_double() const
{
  TYPE_VERIFY(DOUBLE);
  return _as_double();
}


const std::string & node::as_u8str() const
{
  TYPE_VERIFY(U8STR);
  return _as_u8str();
}


array_node & node::as_array()
{
  TYPE_VERIFY(ARRAY);
  return _as_array();
}


const array_node & node::as_array() const
{
  TYPE_VERIFY(ARRAY);
  return _as_array();
}


map_node & node::as_map()
{
  TYPE_VERIFY(MAP);
  return _as_map();
}


const map_node & node::as_map() const
{
  TYPE_VERIFY(MAP);
  return _as_map();
}


binary_node & node::as_binary()
{
  TYPE_VERIFY(BINARY);
  return _as_binary();
}


const binary_node & node::as_binary() const
{
  TYPE_VERIFY(BINARY);
  return _as_binary();
}


node & node::path(const std::string &path)
{
  MIGHTALTER();
  /* Re-use code from const version. */
  return const_cast<node&>(
      static_cast<const node*>(this)->path(path));
}


const node & node::path(const std::string &path) const
{
  enum class parse_state {
    NEED_SELECTOR,
    NEED_INDEX,
    IN_INDEX,
    IN_KEY,
  };

#define BOOM(msg) \
  do { \
    std::ostringstream os; \
    os << "path access failure at position " << (curr - path_start) \
       << " of path " << path << ": " << msg; \
    throw MAKE_ELLIS_ERR(err_code::PATH_ERROR, os.str()); \
  } while (0)

  const node *v = this;
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
          if (! v->is_type(type::MAP)) {
            BOOM("key selector applied to non-map");
          }
          key_start = curr + 1;
          state = parse_state::IN_KEY;
        }
        else if (c == '[') {
          if (! v->is_type(type::ARRAY)) {
            BOOM("index selector applied to non-array");
          }
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
          if (! v->is_type(type::ARRAY)) {
            BOOM("internal error--somehow we have non-array");
          }
          if (index >= v->_as_array().length()) {
            BOOM("index out of range");
          }
          v = &(v->_as_array()[index]);
          state = parse_state::NEED_SELECTOR;
        }
        else {
          BOOM("not a digit");
        }
        break;

      case parse_state::IN_KEY:
        if (c == '}') {
          if (! v->is_type(type::MAP)) {
            BOOM("internal error--somehow we have non-map");
          }
          string key(key_start, curr - key_start);
          if (! v->_as_map().has_key(key)) {
            BOOM("key not found in map");
          }
          v = &(v->_as_map()[key]);
          state = parse_state::NEED_SELECTOR;
        }
        else { /* A character in the key. */
          /* Do nothing: we have a pointer to start of key; wait for end. */
        }
        break;

    }  // switch(state)

  }  // for

  if (state != parse_state::NEED_SELECTOR) {
    BOOM("unexpected path termination");
  }

  return *v;
}


}  /* namespace ellis */
