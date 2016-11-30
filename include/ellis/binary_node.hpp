/*
 * @file binary_node.hpp
 *
 * @brief Ellis binary node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_BINARY_NODE_HPP_
#define ELLIS_BINARY_NODE_HPP_

#include <ellis/node.hpp>

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
  ~binary_node();

  /** Return a reference to the value with the given key.
   *
   * Will throw std::out_of_range if out of range.
   */
  uint8_t & operator[](size_t);
  const uint8_t & operator[](size_t) const;

  /** Binary byte-for-byte comparison; length must be equal also. */
  bool operator==(const binary_node &) const;

  /** Append binary data, extending length as needed. */
  void append(const uint8_t *data, size_t len);

  /** Set the length.  Will truncate if less than current length, or fill
   * with zero bytes if greater than current length.
   */
  void resize(size_t);

  /** Return a pointer to the binary data. */
  uint8_t *data();
  const uint8_t *data() const;

  /** Return the length of the binary data. */
  size_t length() const;

  /** Return true iff length == 0. */
  bool empty() const;

  /** Remove all data. */
  void clear();
};


}  /* namespace ellis */

#endif  /* ELLIS_BINARY_NODE_HPP_ */
