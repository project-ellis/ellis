#include <ellis/binary_node.hpp>

#include <ellis/node.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


#define GETBIN m_node.m_pay->m_bin


binary_node::~binary_node()
{
  assert(0);
  /* This function will never be called; binary_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see binary_node by reference, and only ever be able to destroy
   * a node. */
}


uint8_t& binary_node::operator[](size_t index)
{
  return GETBIN[index];
}


const uint8_t& binary_node::operator[](size_t index) const
{
  return GETBIN[index];
}


bool binary_node::operator==(const binary_node &o) const
{
  return GETBIN == o.GETBIN;
}


void binary_node::append(const uint8_t *data, size_t len)
{
  GETBIN.insert(GETBIN.end(), data, data+len);
}


void binary_node::resize(size_t n)
{
  GETBIN.resize(n);
}


uint8_t * binary_node::data()
{
  return GETBIN.data();
}


const uint8_t * binary_node::data() const
{
  return GETBIN.data();
}


size_t binary_node::length() const
{
  return GETBIN.size();
}


bool binary_node::empty() const
{
  return GETBIN.empty();
}


void binary_node::clear()
{
  GETBIN.clear();
}


}  /* namespace ellis */
