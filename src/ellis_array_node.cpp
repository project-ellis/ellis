#include <ellis/ellis_array_node.hpp>

#include <ellis/ellis_node.hpp>
#include <ellis/private/using.hpp>

namespace ellis {

ellis_array_node::~ellis_array_node()
{
  /* Do nothing; destruction is handled by ellis_node. */
}


ellis_node& ellis_array_node::operator[](size_t index)
{
  return m_node.m_blk->m_arr.operator[](index);
}


const ellis_node& ellis_array_node::operator[](size_t index) const
{
  return m_node.m_blk->m_arr.operator[](index);
}


void ellis_array_node::append(const ellis_node &node)
{
  m_node.m_blk->m_arr.push_back(node);
}


void ellis_array_node::append(ellis_node &&node)
{
  m_node.m_blk->m_arr.push_back(node);
}


void ellis_array_node::extend(const ellis_array_node &other)
{
  m_node.m_blk->m_arr.insert(
    m_node.m_blk->m_arr.cend(),
    other.m_node.m_blk->m_arr.cbegin(),
    other.m_node.m_blk->m_arr.cend());
}


void ellis_array_node::extend(ellis_array_node &&other)
{
  m_node.m_blk->m_arr.insert(
    m_node.m_blk->m_arr.end(),
    other.m_node.m_blk->m_arr.begin(),
    other.m_node.m_blk->m_arr.end());
}


void ellis_array_node::insert(size_t pos, const ellis_node &other)
{
  m_node.m_blk->m_arr.insert(m_node.m_blk->m_arr.cbegin() + pos, other);
}


void ellis_array_node::insert(size_t pos, ellis_node &&other)
{
  m_node.m_blk->m_arr.insert(m_node.m_blk->m_arr.begin() + pos, other);
}


void ellis_array_node::erase(size_t pos)
{
  m_node.m_blk->m_arr.erase(m_node.m_blk->m_arr.begin() + pos);
}


void ellis_array_node::reserve(size_t n)
{
  m_node.m_blk->m_arr.reserve(n);
}


void ellis_array_node::foreach(std::function<void(ellis_node &)> fn)
{
  for (ellis_node &node : m_node.m_blk->m_arr) {
    fn(node);
  }
}


void ellis_array_node::foreach(std::function<void(const ellis_node &)> fn) const
{
  for (const ellis_node &node : m_node.m_blk->m_arr) {
    fn(node);
  }
}


ellis_array_node ellis_array_node::filter(std::function<bool(const ellis_node &)> fn) const
{
  ellis_node res_node(ellis_type::ARRAY);
  /* TODO: add unsafe version of as_array and other functions and use it */
  ellis_array_node &res_arr = res_node.as_array();
  for (ellis_node &node : m_node.m_blk->m_arr) {
    if (fn(node)) {
      res_arr.append(node);
    }
  }

  /* TODO: is there a way to directly return res_arr? i shouldn't have to
   * re-call the function, but res_arr is indeed a local reference... */
  return res_node.as_array();
}


size_t ellis_array_node::length() const
{
  return m_node.m_blk->m_arr.size();
}


bool ellis_array_node::empty() const
{
  return m_node.m_blk->m_arr.empty();
}


void ellis_array_node::clear()
{
  m_node.m_blk->m_arr.clear();
}


}  /* namespace ellis */
