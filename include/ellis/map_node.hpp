/*
 * @file map_node.hpp
 *
 * @brief Ellis map node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_MAP_NODE_HPP_
#define ELLIS_MAP_NODE_HPP_

#include <string>
#include <vector>
#include <ellis/node.hpp>

namespace ellis {


struct merge_policy {
  bool key_exists_copy;
  bool key_missing_copy;
  bool abort_on_not_copy;
};


/** Typesafe map wrap.
 *
 * Operations on an map_node assume that the node has been
 * validated as a map by node::as_map().
 */
class map_node {
  /* Contains a single data member--an node--and no vtable or
   * other wrapping.  This is important since we plan to cast between the two
   * in order to use this shell class for type safety. */
  node m_node;

public:
  /** Constructor
   */
  map_node() = delete;
  ~map_node();

  /** Return a reference to the value with the given key.
   *
   * Will throw std::out_of_range if key is not present.
   */
  node & operator[](const std::string &);
  const node & operator[](const std::string &) const;

  /** Map contents comparison.  Same keys, same values. */
  bool operator==(const map_node &) const;

  /** Add a new value with the given key name. */
  void insert(const std::string &, const node &);
  void insert(const std::string &, node &&);

  /** Merge the contents of another map, using the given policy. */
  void merge(const map_node &other, const merge_policy &policy);

  /** Remove the given key and corresponding value from the map.
   *
   * If key is not present, do nothing (not an error).
   */
  void erase(const std::string &);

  /** Return true iff the map has a key of the given name. */
  bool has_key(const std::string &) const;

  /** Return the keys found in the map. */
  std::vector<std::string> keys() const;

  /** Run the specified function on each key/value entry in the map. */
  void foreach(std::function<void(const std::string &, node &)> fn);
  void foreach(std::function<
      void(const std::string &, const node &)> fn) const;

  /** Select entries in the map matching given criteria.
   *
   * The result is a new map (with entries copy on write).
   */
  node filter(std::function<
      bool(const std::string &, const node &)> fn) const;

  /** Return number of keys in map. */
  size_t length() const;

  /** Return true iff there are no entries in the map. */
  bool empty() const;

  /** Remove all entries in the map. */
  void clear();
};


}  /* namespace ellis */

#endif  /* ELLIS_MAP_NODE_HPP_ */
