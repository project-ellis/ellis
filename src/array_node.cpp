#include <ellis/array_node.hpp>

#include <ellis/node.hpp>
#include <ellis/private/using.hpp>

namespace ellis {

array_node::~array_node()
{
  assert(0);
  /* This function will never be called; array_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see array_node by reference, and only ever be able to destroy
   * a node. */
}


node& array_node::operator[](size_t index)
{
  return m_node.m_blk->m_arr[index];
}


const node& array_node::operator[](size_t index) const
{
  return m_node.m_blk->m_arr[index];
}


void array_node::append(const node &node)
{
  m_node.m_blk->m_arr.push_back(node);
}


void array_node::append(node &&node)
{
  m_node.m_blk->m_arr.push_back(node);
}


void array_node::extend(const array_node &other)
{
  m_node.m_blk->m_arr.insert(
    m_node.m_blk->m_arr.cend(),
    other.m_node.m_blk->m_arr.cbegin(),
    other.m_node.m_blk->m_arr.cend());
}


void array_node::extend(array_node &&other)
{
  m_node.m_blk->m_arr.insert(
    m_node.m_blk->m_arr.end(),
    other.m_node.m_blk->m_arr.begin(),
    other.m_node.m_blk->m_arr.end());
}


void array_node::insert(size_t pos, const node &other)
{
  m_node.m_blk->m_arr.insert(m_node.m_blk->m_arr.cbegin() + pos, other);
}


void array_node::insert(size_t pos, node &&other)
{
  m_node.m_blk->m_arr.insert(m_node.m_blk->m_arr.begin() + pos, other);
}


void array_node::erase(size_t pos)
{
  m_node.m_blk->m_arr.erase(m_node.m_blk->m_arr.begin() + pos);
}


void array_node::reserve(size_t n)
{
  m_node.m_blk->m_arr.reserve(n);
}


void array_node::foreach(std::function<void(node &)> fn)
{
  for (node &node : m_node.m_blk->m_arr) {
    fn(node);
  }
}


void array_node::foreach(std::function<void(const node &)> fn) const
{
  for (const node &node : m_node.m_blk->m_arr) {
    fn(node);
  }
}


node array_node::filter(std::function<bool(const node &)> fn) const
{
  node res_node(type::ARRAY);
  array_node &res_arr = res_node._as_array();
  for (node &node : m_node.m_blk->m_arr) {
    if (fn(node)) {
      res_arr.append(node);
    }
  }

  return res_node;
}


size_t array_node::length() const
{
  return m_node.m_blk->m_arr.size();
}


bool array_node::empty() const
{
  return m_node.m_blk->m_arr.empty();
}


void array_node::clear()
{
  m_node.m_blk->m_arr.clear();
}


}  /* namespace ellis */
