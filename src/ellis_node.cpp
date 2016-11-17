#include <ellis/ellis_node.hpp>

#include <ellis/ellis_err.hpp>
#include <ellis/private/using.hpp>
#include <stddef.h>
#include <string.h>
#include <strings.h>

namespace ellis {


/** A convenience macro for throwing an error when type is improper. */
#define TYPE_VERIFY(typ) \
  do { \
    if (get_type() != ellis_type::typ) { \
      throw MAKE_ELLIS_ERR(ellis_err_code::WRONG_TYPE, "not " #typ); \
    } \
  } while (0)


/** Initialize contents to pristine state, assuming no prior contents.
 *
 * If m_type indicates a pointer in the union, it will be set up;
 * or if an in-place constructor is needed, it will be called.
 */
void ellis_node::_zap_contents(ellis_type t)
{
  m_type = (int)t;
  bool wantcount = true;
  bzero(m_pad,  sizeof(m_pad));
  switch (ellis_type(m_type)) {
    case ellis_type::BOOL:
    case ellis_type::DOUBLE:
    case ellis_type::INT64:
    case ellis_type::NIL:
      wantcount = false;
      break;

    case ellis_type::ARRAY:
      m_arr = new arr_t();
      break;

    case ellis_type::BINARY:
      m_bin = new bin_t();
      break;

    case ellis_type::MAP:
      m_map = new map_t();
      break;

    case ellis_type::U8STR:
      wantcount = false;
      new (&m_str) std::string();
      break;
  }
  if (wantcount) {
    m_refcount = new refcount_t(1);
  } else {
    m_refcount = nullptr;
  }
}


/** Grab state from other node, assuming no prior state.
 *
 * If the node already has contents, caller should call release_contents()
 * before calling this function.
 */
void ellis_node::_grab_contents(const ellis_node& other)
{
  static_assert(
      sizeof(m_pad) == offsetof(ellis_node, m_refcount),
      "Uh oh, m_pad does not seem to cover entire union...");
  m_type = other.m_type;
  m_refcount = other.m_refcount;
  if (m_refcount != nullptr) {
    (*m_refcount)++;
  }
  switch (ellis_type(m_type)) {
    case ellis_type::ARRAY:
    case ellis_type::BINARY:
    case ellis_type::BOOL:
    case ellis_type::DOUBLE:
    case ellis_type::INT64:
    case ellis_type::MAP:
    case ellis_type::NIL:
      memcpy(m_pad, other.m_pad, sizeof(m_pad));
      break;

    case ellis_type::U8STR:
      /* In-place string copy constructor. */
      new (&m_str) std::string(other.m_str);
      break;
  }
}


/** Release the contents.
 *
 * It is safe to call _release_contents multiple times, which has the same
 * effect as calling it once.
 */
void ellis_node::_release_contents()
{
  /* String is a special case; if type==u8str, destroy no matter refcount. */
  if (ellis_type(m_type) == ellis_type::U8STR) {
    m_str.~basic_string<char>();
  }

  /* If refcount is null, our prize has been pilfered; nothing to do. */
  if (m_refcount != nullptr) {
    (*m_refcount)--;
    if (*m_refcount == 0) {
      switch (ellis_type(m_type)) {
        case ellis_type::BOOL:
        case ellis_type::DOUBLE:
        case ellis_type::INT64:
        case ellis_type::NIL:
        case ellis_type::U8STR:
          /* Nothing to do. */
          break;

        case ellis_type::ARRAY:
          delete m_arr;
          break;

        case ellis_type::BINARY:
          delete m_bin;
          break;

        case ellis_type::MAP:
          delete m_map;
          break;
      }
      delete m_refcount;
      m_refcount = nullptr;
    }
  }
  m_type = (int)ellis_type::NIL;
}


ellis_node::ellis_node(ellis_type t)
{
  _zap_contents(t);
}


ellis_node::ellis_node(const uint8_t *mem, size_t bytes)
{
  _zap_contents(ellis_type::BINARY);
  m_bin->resize(bytes);
  memcpy(m_bin->data(), mem, bytes);
}


ellis_node::ellis_node(bool b)
{
  _zap_contents(ellis_type::BOOL);
  m_boo = b;
}


ellis_node::ellis_node(const std::string& s)
{
  _zap_contents(ellis_type::U8STR);
  new (&m_str) std::string(s);
}


ellis_node::ellis_node(int64_t i)
{
  _zap_contents(ellis_type::INT64);
  m_int = i;
}


ellis_node::ellis_node(double d)
{
  _zap_contents(ellis_type::DOUBLE);
  m_dbl = d;
}


ellis_node::ellis_node(const ellis_node& other)
{
  _grab_contents(other);
}


ellis_node::ellis_node(ellis_node&& other)
{
  _grab_contents(other);
  other._release_contents();
}


ellis_node::~ellis_node()
{
  _release_contents();
}


void ellis_node::swap(ellis_node &other)
{
  ellis_node tmp(*this);
  *this = other;
  other = tmp;
}


ellis_node& ellis_node::operator=(const ellis_node& rhs)
{
  _release_contents();
  _grab_contents(rhs);
  return *this;
}


ellis_node& ellis_node::operator=(ellis_node&& rhs)
{
  _release_contents();
  _grab_contents(rhs);
  rhs._release_contents();
  return *this;
}


#if 0
bool ellis_node::operator==(const ellis_node &) const
{
}


bool ellis_node::operator!=(const ellis_node &o) const
{
  return not (*this == o)
}
#endif


bool ellis_node::is_type(ellis_type t) const
{
  return get_type() == t;
}


ellis_type ellis_node::get_type() const
{
  return (ellis_type)m_type;
}


ellis_node::operator int64_t() const
{
  return as_int64();
}


ellis_node::operator double() const
{
  return as_double();
}


int64_t ellis_node::as_int64() const
{
  TYPE_VERIFY(INT64);
  return m_int;
}


int64_t ellis_node::as_double() const
{
  TYPE_VERIFY(DOUBLE);
  return m_dbl;
}


const std::string & ellis_node::as_u8str() const
{
  TYPE_VERIFY(U8STR);
  return m_str;
}


#if 0
ellis_array_node & ellis_node::as_array()
{
}


const ellis_array_node & ellis_node::as_array() const
{
  TYPE_VERIFY(ARRAY);
}
#endif


ellis_map_node & ellis_node::as_map()
{
  /* Re-use code from const version. */
  return const_cast<ellis_map_node&>(
      static_cast<const ellis_node*>(this)->as_map());
}


const ellis_map_node & ellis_node::as_map() const
{
  TYPE_VERIFY(MAP);
  return *(reinterpret_cast<const ellis_map_node*>(this));
}


uint8_t* ellis_node::as_binary(size_t *size)
{
  /* Re-use code from const version. */
  return const_cast<uint8_t*>(
      static_cast<const ellis_node*>(this)->as_binary(size));
}


const uint8_t* ellis_node::as_binary(size_t *size) const
{
  TYPE_VERIFY(BINARY);
  *size = m_bin->size();
  return m_bin->data();
}

#if 0
ellis_node & ellis_node::get_path(const std::string &path)
{
}


const ellis_node & ellis_node::get_path(const std::string &path) const
{
}


#endif
}  /* namespace ellis */
