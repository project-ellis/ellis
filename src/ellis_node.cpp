#include <ellis/ellis_node.hpp>

#include <ellis/ellis_err.hpp>
#include <ellis/private/using.hpp>
#include <stddef.h>
#include <string.h>

namespace ellis {


/** A convenience macro for throwing an error when type is improper. */
#define TYPE_VERIFY(typ) \
  do { \
    if (get_type() != type::typ) { \
      throw MAKE_ELLIS_ERR(err_code::WRONG_TYPE, "not " #typ); \
    } \
  } while (0)


bool node::_is_refcounted()
{
  switch (type(m_type)) {
    case type::BOOL:
    case type::DOUBLE:
    case type::INT64:
    case type::NIL:
    case type::U8STR:
      return false;

    case type::ARRAY:
    case type::BINARY:
    case type::MAP:
      return true;
  }
  /* Never reached. */
  assert(0);
  return false;
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
  else if (_is_refcounted()) {
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
  else if (_is_refcounted()) {
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
  else if (_is_refcounted()) {
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


node::node(const std::string& s)
{
  _zap_contents(type::U8STR);
  m_str = s;
}


node::node(int64_t i)
{
  _zap_contents(type::INT64);
  m_int = i;
}


node::node(double d)
{
  _zap_contents(type::DOUBLE);
  m_dbl = d;
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


#if 0
bool node::operator==(const node &) const
{
}


bool node::operator!=(const node &o) const
{
  return not (*this == o)
}
#endif


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


/* TODO: add _ versions of the as_* functions that skip type checks. */


int64_t node::as_int64() const
{
  TYPE_VERIFY(INT64);
  return m_int;
}


int64_t node::as_double() const
{
  TYPE_VERIFY(DOUBLE);
  return m_dbl;
}


const std::string & node::as_u8str() const
{
  TYPE_VERIFY(U8STR);
  return m_str;
}


array_node & node::as_array()
{
  /* Re-use code from const version. */
  return const_cast<array_node&>(
      static_cast<const node*>(this)->as_array());
}


const array_node & node::as_array() const
{
  TYPE_VERIFY(ARRAY);
  return *(reinterpret_cast<const array_node*>(this));
}


map_node & node::as_map()
{
  /* Re-use code from const version. */
  return const_cast<map_node&>(
      static_cast<const node*>(this)->as_map());
}


const map_node & node::as_map() const
{
  TYPE_VERIFY(MAP);
  return *(reinterpret_cast<const map_node*>(this));
}


uint8_t* node::as_binary(size_t *size)
{
  /* Re-use code from const version. */
  return const_cast<uint8_t*>(
      static_cast<const node*>(this)->as_binary(size));
}


const uint8_t* node::as_binary(size_t *size) const
{
  TYPE_VERIFY(BINARY);
  *size = m_blk->m_bin.size();
  return m_blk->m_bin.data();
}

#if 0
node & node::get_path(const std::string &path)
{
}


const node & node::get_path(const std::string &path) const
{
}


#endif
}  /* namespace ellis */
