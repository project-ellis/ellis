/*
 * @file ellis/core/binary_node.hpp
 *
 * @brief Ellis binary node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_CORE_BINARY_NODE_HPP_
#define ELLIS_CORE_BINARY_NODE_HPP_

#include <ellis/core/node.hpp>

namespace ellis {


/** Typesafe binary data wrapper.
 *
 * Operations on an binary_node assume that the node has been
 * validated as binary by node::as_binary().
 */
class binary_node {
  /* Contains a single data member--an node--and no vtable or
   * other wrapping.  This is important since we plan to cast between the two
   * in order to use this shell class for type safety. */
  node m_node;

public:
  /** Constructor
   */
  binary_node() = delete;
  binary_node(const binary_node &) = delete;
  binary_node(binary_node &&) = delete;
  ~binary_node();

  /** Return a reference to the value with the given key.
   *
   * Will throw std::out_of_range if out of range.
   */
  byte & operator[](size_t);
  const byte & operator[](size_t) const;

  /** Binary byte-for-byte comparison; length must be equal also. */
  bool operator==(const binary_node &) const;

  /** Append binary data, extending length as needed. */
  void append(const byte *data, size_t len);

  /** Set the length.  Will truncate if less than current length, or fill
   * with zero bytes if greater than current length.
   */
  void resize(size_t);

  /** Return a pointer to the binary data. */
  byte *data();
  const byte *data() const;

  /** Return the length of the binary data. */
  size_t length() const;

  /** Return true iff length == 0. */
  bool is_empty() const;

  /** Remove all data. */
  void clear();
};


}  /* namespace ellis */

#endif  /* ELLIS_CORE_BINARY_NODE_HPP_ */
