#include <ellis/core/u8str_node.hpp>

#include <ellis/core/node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/core/payload.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


#define GETSTR m_node.m_pay->m_str


u8str_node::~u8str_node()
{
  ELLIS_ASSERT_UNREACHABLE();
  /* This function should never be called; u8str_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see u8str_node by reference, and only be able to destroy
   * a node, not a u8str_node. */
}


char& u8str_node::operator[](size_t index)
{
  return GETSTR[index];
}


const char& u8str_node::operator[](size_t index) const
{
  return GETSTR[index];
}


bool u8str_node::operator==(const u8str_node &o) const
{
  return GETSTR == o.GETSTR;
}


void u8str_node::assign(const char *src_str)
{
  GETSTR.assign(src_str);
}


void u8str_node::assign(const char *src_str, size_t len)
{
  GETSTR.assign(src_str, len);
}


void u8str_node::append(const char *src_str)
{
  GETSTR.append(src_str);
}


void u8str_node::append(const char *src_str, size_t len)
{
  GETSTR.append(src_str, len);
}


void u8str_node::resize(size_t n)
{
  GETSTR.resize(n);
}


const char * u8str_node::c_str() const
{
  return GETSTR.c_str();
}


size_t u8str_node::length() const
{
  return GETSTR.size();
}


bool u8str_node::is_empty() const
{
  return GETSTR.empty();
}


void u8str_node::clear()
{
  GETSTR.clear();
}


std::ostream & operator<<(std::ostream & os, const u8str_node &v)
{
  os << v.c_str();
  return os;
}


}  /* namespace ellis */
