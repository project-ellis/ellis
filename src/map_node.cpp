#include <ellis/map_node.hpp>

#include <ellis/err.hpp>
#include <ellis/node.hpp>
#include <ellis/private/payload.hpp>
#include <ellis/private/using.hpp>


namespace ellis {


#define GETMAP m_node.m_pay->m_map


map_node::~map_node()
{
  assert(0);
  /* This function will never be called; map_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see map_node by reference, and only ever be able to destroy
   * a node. */
}


node & map_node::operator[](const std::string &key)
{
  return const_cast<node&>(
      (*(static_cast<const map_node*>(this)))[key]);
}


const node & map_node::operator[](const std::string &key) const
{
  const auto it = GETMAP.find(key);
  if (it == GETMAP.end()) {
    auto added = GETMAP.emplace(key, node(type::NIL));
    assert(added.second == true);
    return added.first->second;
  }
  return it->second;
}


bool map_node::operator==(const map_node &o) const
{
  return GETMAP == o.GETMAP;
}


void map_node::insert(const std::string &key, const node &val)
{
  GETMAP.emplace(key, val);
}


void map_node::insert(const std::string &key, node &&val)
{
  GETMAP.emplace(key, std::move(val));
}


void map_node::merge(const map_node &other, const merge_policy &policy)
{
  for (const auto &it : other.m_node.m_pay->m_map) {
    bool exists = GETMAP.count(it.first);
    bool q_replace = exists && policy.key_exists_copy;
    bool q_insert = (not exists) && policy.key_missing_copy;
    /* q_replace and q_insert can not both be set. */
    assert(! (q_replace && q_insert));
    if (q_insert) {
      insert(it.first, it.second);
    }
    if (q_replace) {
      erase(it.first);
      insert(it.first, it.second);
    }
    if ((!q_insert) && (!q_replace) && policy.abort_on_not_copy) {
      /* Failed copy criteria; abort because policy says so. */
      throw MAKE_ELLIS_ERR(err_code::NOT_MERGED, it.first + " not merged");
    }
  }
}


void map_node::erase(const std::string &key)
{
  GETMAP.erase(key);
}


bool map_node::has_key(const std::string &key) const
{
  return GETMAP.count(key) > 0;
}


std::vector<std::string> map_node::keys() const
{
  vector<string> rv;
  for (const auto &it : GETMAP) {
    rv.push_back(it.first);
  }
  return rv;
}


void map_node::foreach(std::function<void(const std::string &, node &)> fn)
{
  for (auto &it : GETMAP) {
    fn(it.first, it.second);
  }
}


void map_node::foreach(std::function<
    void(const std::string &, const node &)> fn) const
{
  for (const auto &it : GETMAP) {
    fn(it.first, it.second);
  }
}


node map_node::filter(std::function<
    bool(const std::string &, const node &)> fn) const
{
  node res_node(type::MAP);
  auto &res_map = res_node._as_mutable_map();
  for (const auto &it : GETMAP) {
    if (fn(it.first, it.second)) {
      res_map.insert(it.first, it.second);
    }
  }
  return res_node;
}


size_t map_node::length() const
{
  return GETMAP.size();
}


bool map_node::empty() const
{
  return GETMAP.empty();
}


void map_node::clear()
{
  GETMAP.clear();
}


}  /* namespace ellis */
