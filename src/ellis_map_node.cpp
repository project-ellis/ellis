#include <ellis/ellis_map_node.hpp>

#include <ellis/ellis_node.hpp>
#include <ellis/private/using.hpp>


namespace ellis {


map_node::~map_node()
{
  /* Do nothing; destruction is handled by node. */
}


node & map_node::operator[](const std::string &key)
{
  return const_cast<node&>(
      (*(static_cast<const map_node*>(this)))[key]);
}


const node & map_node::operator[](const std::string &key) const
{
  const auto it = m_node.m_blk->m_map.find(key);
  if (it == m_node.m_blk->m_map.end()) {
    auto added = m_node.m_blk->m_map.emplace(key, node(type::NIL));
    assert(added.second == true);
    return added.first->second;
  }
  return it->second;
}


void map_node::insert(const std::string &key, const node &val)
{
  m_node.m_blk->m_map.emplace(key, val);
}


void map_node::insert(const std::string &key, node &&val)
{
  m_node.m_blk->m_map.emplace(key, std::move(val));
}


}  /* namespace ellis */
