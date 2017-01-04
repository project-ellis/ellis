/*
 * @file ellis/core/u8str_node.hpp
 *
 * @brief Ellis u8str node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_CORE_U8STR_NODE_HPP_
#define ELLIS_CORE_U8STR_NODE_HPP_

#include <ellis/core/node.hpp>

namespace ellis {


/**
 * Typesafe u8str data wrapper.
 *
 * Operations on an u8str_node assume that the node has been
 * validated as u8str by node::as_u8str().
 */
class u8str_node {
  /*
   * Contains a single data member--an node--and no vtable or
   * other wrapping.  This is important since we plan to cast between the two
   * in order to use this shell class for type safety.
   */
  node m_node;

public:
  /**
   * Constructor
   */
  u8str_node() = delete;
  u8str_node(const u8str_node &) = delete;
  u8str_node(u8str_node &&) = delete;
  ~u8str_node();

  /** 
   * Extract a character from the string.
   *
   * Will throw std::out_of_range if out of range.
   */
  char & operator[](size_t);
  const char & operator[](size_t) const;

  /**
   * Simple char-for-char comparison; length must be equal also.
   */
  bool operator==(const u8str_node &) const;

  /**
   * Assign contents to the provided string, up to null terminator.
   */
  void assign(const char *str);

  /**
   * Assign len chars from the provided string, not to exceed null terminator.
   *
   * TODO: code needs to stop at null terminator.
   */
  void assign(const char *str, size_t len);

  /**
   * Append additional characters up to null terminator, extending internal
   * length as needed.
   */
  void append(const char *str);

  /**
   * Append len additional characters (not to exceed null terminator),
   * extending internal length as needed.
   *
   * TODO: code needs to stop at null terminator.
   */
  void append(const char *str, size_t len);

  /**
   * Set the length.  Will truncate if less than current length, or fill
   * with null characters if greater than current length.
   */
  void resize(size_t);

  /**
   * Return a pointer to the UTF-8 encoded null-terminated string data.
   */
  const char *c_str() const;

  /**
   * Return the length of the string.
   */
  size_t length() const;

  /**
   * Return true iff length == 0.
   */
  bool is_empty() const;

  /**
   * Clear the contents, i.e. set equal to empty string.
   */
  void clear();
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


std::ostream & operator<<(std::ostream & os, const u8str_node & v);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_U8STR_NODE_HPP_ */
