#include <ellis/array_node.hpp>

#include <ellis/node.hpp>
#include <ellis/private/using.hpp>

namespace ellis {


#define GETARR m_node.m_blk->m_arr


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
  return GETARR[index];
}


const node& array_node::operator[](size_t index) const
{
  return GETARR[index];
}


bool array_node::operator==(const array_node &o) const
{
  return GETARR == o.GETARR;
}


void array_node::append(const node &node)
{
  GETARR.push_back(node);
}


void array_node::append(node &&node)
{
  GETARR.push_back(node);
}


void array_node::extend(const array_node &other)
{
  GETARR.insert(
    GETARR.cend(),
    other.GETARR.cbegin(),
    other.GETARR.cend());
}


void array_node::extend(array_node &&other)
{
  GETARR.insert(
    GETARR.end(),
    other.GETARR.begin(),
    other.GETARR.end());
}


void array_node::insert(size_t pos, const node &other)
{
  GETARR.insert(GETARR.cbegin() + pos, other);
}


void array_node::insert(size_t pos, node &&other)
{
  GETARR.insert(GETARR.begin() + pos, other);
}


void array_node::erase(size_t pos)
{
  GETARR.erase(GETARR.begin() + pos);
}


void array_node::reserve(size_t n)
{
  GETARR.reserve(n);
}


void array_node::foreach(std::function<void(node &)> fn)
{
  for (node &node : GETARR) {
    fn(node);
  }
}


void array_node::foreach(std::function<void(const node &)> fn) const
{
  for (const node &node : GETARR) {
    fn(node);
  }
}


node array_node::filter(std::function<bool(const node &)> fn) const
{
  node res_node(type::ARRAY);
  array_node &res_arr = res_node._as_mutable_array();
  for (node &node : GETARR) {
    if (fn(node)) {
      res_arr.append(node);
    }
  }

  return res_node;
}


size_t array_node::length() const
{
  return GETARR.size();
}


bool array_node::empty() const
{
  return GETARR.empty();
}


void array_node::clear()
{
  GETARR.clear();
}


}  /* namespace ellis */
