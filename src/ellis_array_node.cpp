#include <ellis/ellis_array_node.hpp>

#include <ellis/ellis_node.hpp>
#include <ellis/private/using.hpp>

namespace ellis {

array_node::~array_node()
{
  /* Do nothing; destruction is handled by node. */
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


array_node array_node::filter(std::function<bool(const node &)> fn) const
{
  node res_node(type::ARRAY);
  /* TODO: add unsafe version of as_array and other functions and use it */
  array_node &res_arr = res_node.as_array();
  for (node &node : m_node.m_blk->m_arr) {
    if (fn(node)) {
      res_arr.append(node);
    }
  }

  /* TODO: is there a way to directly return res_arr? i shouldn't have to
   * re-call the function, but res_arr is indeed a local reference... */
  return res_node.as_array();
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
