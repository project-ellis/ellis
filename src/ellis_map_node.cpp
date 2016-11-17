#include <ellis/ellis_map_node.hpp>

#include <ellis/ellis_node.hpp>
#include <ellis/private/using.hpp>


namespace ellis {


ellis_map_node::~ellis_map_node()
{
  /* Do nothing; destruction is handled by ellis_node. */
}


ellis_node & ellis_map_node::operator[](const std::string &key)
{
  return const_cast<ellis_node&>(
      (*(static_cast<const ellis_map_node*>(this)))[key]);
}


const ellis_node & ellis_map_node::operator[](const std::string &key) const
{
  const auto it = m_node.m_blk->m_map.find(key);
  if (it == m_node.m_blk->m_map.end()) {
    auto added = m_node.m_blk->m_map.emplace(key, ellis_node(ellis_type::NIL));
    assert(added.second == true);
    return added.first->second;
  }
  return it->second;
}


void ellis_map_node::insert(const std::string &key, const ellis_node &val)
{
  m_node.m_blk->m_map.emplace(key, val);
}


void ellis_map_node::insert(const std::string &key, ellis_node &&val)
{
  m_node.m_blk->m_map.emplace(key, std::move(val));
}


}  /* namespace ellis */
