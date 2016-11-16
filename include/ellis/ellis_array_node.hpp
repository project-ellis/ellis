/*
 * @file ellis_array_node.hpp
 *
 * @brief Ellis array node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_ARRAY_NODE_HPP_
#define ELLIS_ARRAY_NODE_HPP_

#include <ellis/ellis_type.hpp>

namespace ellis {


/** Typesafe array wrap.
 *
 * Operations on an ellis_array_node assume that the ellis_node has been
 * validated as an array by ellis_node::as_array().
 */
class ellis_array_node {
  /* Contains a single data member--an ellis_node--and no vtable or
   * other wrapping.  This is important since we plan to cast between the two
   * in order to use this shell class for type safety. */
  ellis_node node;

public:
  /** Return a reference to the element at the given index.
   *
   * Will throw std::out_of_range if key is not present.
   */
  ellis_node& operator[](size_t);
  const ellis_node& operator[](size_t) const;

  /** Append a node to the end of the array.
   */
  void append(const ellis_node &node);
  void append(ellis_node &&node);

  /** Extend array by appending the contents of another array.
   */
  void extend(const ellis_array_node &other);
  void extend(ellis_array_node &&other);

  /** Insert a new element at the given position. */
  void insert(size_t position, const ellis_node &);
  void insert(size_t position, ellis_node &&);

  /** Remove element at the given position. */
  void erase(size_t position);

  /** Reserve space for n elements in the vector, without actually changing
   * the length.  Has no effect if n is less than or equal to current length. */
  void reserve(size_t n);

  /** Run the specified function on each element in the array. */
  void foreach(std::function<void(ellis_node &)> fn);
  void foreach(std::function<void(const ellis_node &)> fn) const;

  /** Select elements in the array matching given criteria.
   *
   * The result is a new array (with elements copy on write).
   */
  ellis_array_node & filter(std::function<bool(const ellis_node &)> fn) const;

  /** Return number of elements in array. */
  size_t length() const;

  /** Return true iff there are no elements in the array (length==0). */
  bool empty() const;

  /** Remove all elements from the array. */
  void clear();
};


/** Policy for merges to ellis maps. */
struct ellis_merge_policy {
  bool key_exists_hit;
  bool key_missing_hit;
  bool abort_on_miss;
};


/*  ___          _
 * |_ _|___  ___| |_ _ __ ___  __ _ _ __ ___
 *  | |/ _ \/ __| __| '__/ _ \/ _` | '_ ` _ \
 *  | | (_) \__ \ |_| | |  __/ (_| | | | | | |
 * |___\___/|___/\__|_|  \___|\__,_|_| |_| |_|
 *
 *   ___                       _
 *  / _ \ _ __   ___ _ __ __ _| |_ ___  _ __ ___
 * | | | | '_ \ / _ \ '__/ _` | __/ _ \| '__/ __|
 * | |_| | |_) |  __/ | | (_| | || (_) | |  \__ \
 *  \___/| .__/ \___|_|  \__,_|\__\___/|_|  |___/
 *       |_|
 */


std::ostream & operator<<(std::ostream & os, const ellis_node & v);
std::istream & operator>>(std::istream & is, ellis_node & v);


} /* namespace ellis */

#endif /* ELLIS_ARRAY_NODE_HPP_ */
