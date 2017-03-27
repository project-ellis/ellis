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
