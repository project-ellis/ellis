/*
 * Copyright (c) 2016 Surround.IO Corporation. All Rights Reserved.
 * Copyright (c) 2017 Xevo Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <ellis/core/map_node.hpp>

#include <ellis/core/err.hpp>
#include <ellis/core/node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/core/payload.hpp>
#include <ellis_private/using.hpp>


namespace ellis {


#define GETMAP m_node.m_pay->m_map


map_node::~map_node()
{
  ELLIS_ASSERT_UNREACHABLE();
  /* This function should never be called; map_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see map_node by reference, and only be able to destroy
   * a node, not a map_node. */
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
    ELLIS_ASSERT(added.second);
    return added.first->second;
  }
  return it->second;
}


bool map_node::operator==(const map_node &o) const
{
  return GETMAP == o.GETMAP;
}


void map_node::add(
    const std::string &key,
    const node &val,
    add_policy addpol,
    add_failure_fn *failfn)
{
  auto it = GETMAP.find(key);
  bool exists = not (it == GETMAP.end());
  bool will_replace = exists && addpol != add_policy::INSERT_ONLY;
  bool will_insert = (not exists) && addpol != add_policy::REPLACE_ONLY;
  /* will_replace and will_insert can not both be set. */
  ELLIS_ASSERT(! (will_replace && will_insert));

  if (will_insert) {
    GETMAP.emplace(key, val);
  }
  else if (will_replace) {
    GETMAP.erase(it->first);
    GETMAP.emplace(key, val);
  }
  else {
    if (failfn != nullptr) {
      (*failfn)(key, val);
    }
  }
}


void map_node::add(
    const char *key,
    const node &val,
    add_policy addpol,
    add_failure_fn *failfn)
{
  add(string(key), val, addpol, failfn);
}


void map_node::insert(const std::string &key, const node &val)
{
  add(key, val, add_policy::INSERT_ONLY, nullptr);
}


void map_node::insert(const char *key, const node &val)
{
  add(key, val, add_policy::INSERT_ONLY, nullptr);
}


void map_node::replace(const std::string &key, const node &val)
{
  add(key, val, add_policy::REPLACE_ONLY, nullptr);
}


void map_node::replace(const char *key, const node &val)
{
  add(key, val, add_policy::REPLACE_ONLY, nullptr);
}


void map_node::set(const std::string &key, const node &val)
{
  add(key, val, add_policy::INSERT_OR_REPLACE, nullptr);
}


void map_node::set(const char *key, const node &val)
{
  add(key, val, add_policy::INSERT_OR_REPLACE, nullptr);
}


void map_node::merge(
    const map_node &other,
    add_policy addpol,
    add_failure_fn *failfn)
{
  for (const auto &it : other.m_node.m_pay->m_map) {
    add(it.first, it.second, addpol, failfn);
  }
}


void map_node::erase(const std::string &key)
{
  GETMAP.erase(key);
}


void map_node::erase(const char *key)
{
  GETMAP.erase(key);
}


bool map_node::has_key(const char *key) const
{
  return GETMAP.count(key) > 0;
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


void map_node::foreach_mutable(std::function<
    void(const std::string &, node &)> fn)
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


bool map_node::is_empty() const
{
  return GETMAP.empty();
}


void map_node::clear()
{
  GETMAP.clear();
}


std::ostream & operator<<(std::ostream & os, const map_node &v)
{
  size_t count = 0;
  size_t length = v.length();
  os << "{";
  v.foreach([&os, &count, &length](const string &k, const node &n)
  {
    os << k << ": " << n;
    count++;
    if (count < length) {
      os << ", ";
    }
  });
  os << "}";

  return os;
}


}  /* namespace ellis */
