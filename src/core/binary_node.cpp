#include <ellis/binary_node.hpp>

#include <ellis/node.hpp>
#include <ellis/private/payload.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


#define GETBIN m_node.m_pay->m_bin


binary_node::~binary_node()
{
  assert(0);
  /* This function should never be called; binary_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see binary_node by reference, and only be able to destroy
   * a node, not a binary_node. */
}


byte& binary_node::operator[](size_t index)
{
  return GETBIN[index];
}


const byte& binary_node::operator[](size_t index) const
{
  return GETBIN[index];
}


bool binary_node::operator==(const binary_node &o) const
{
  return GETBIN == o.GETBIN;
}


void binary_node::append(const byte *srcdata, size_t len)
{
  GETBIN.insert(GETBIN.end(), srcdata, srcdata+len);
}


void binary_node::resize(size_t n)
{
  GETBIN.resize(n);
}


byte * binary_node::data()
{
  return GETBIN.data();
}


const byte * binary_node::data() const
{
  return GETBIN.data();
}


size_t binary_node::length() const
{
  return GETBIN.size();
}


bool binary_node::is_empty() const
{
  return GETBIN.empty();
}


void binary_node::clear()
{
  GETBIN.clear();
}


}  /* namespace ellis */
