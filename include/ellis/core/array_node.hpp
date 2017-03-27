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
 * @file ellis/core/array_node.hpp
 *
 * @brief Ellis array node public C++ header.
 *
 */

#pragma once
#ifndef ELLIS_CORE_ARRAY_NODE_HPP_
#define ELLIS_CORE_ARRAY_NODE_HPP_

#include <ellis/core/node.hpp>
#include <functional>

namespace ellis {


/** Typesafe array wrap.
 *
 * Operations on an array_node assume that the node has been
 * validated as an array by node::as_array().
 */
class array_node {
  /* Contains a single data member--an node--and no vtable or
   * other wrapping.  This is important since we plan to cast between the two
   * in order to use this shell class for type safety. */
  node m_node;

public:
  /** Constructor
   */
  array_node() = delete;
  array_node(const array_node &) = delete;
  array_node(array_node &&) = delete;
  ~array_node();

  /** Return a reference to the element at the given index.
   *
   * Will throw std::out_of_range if index is out of bounds.
   */
  node& operator[](size_t index);
  const node& operator[](size_t index) const;

  /** Return true iff lengths same and corresponding elements same. */
  bool operator==(const array_node &) const;

  /** Append a node to the end of the array.
   */
  void append(const node &node);

  /** Extend array by appending the contents of another array.
   */
  void extend(const array_node &other);

  /** Insert a new element at the given position. */
  void insert(size_t pos, const node &);

  /** Remove element at the given position. */
  void erase(size_t pos);

  /** Reserve space for n elements in the array, without actually changing
   * the length.
   *
   * Has no effect if n is less than or equal to current length.
   */
  void reserve(size_t n);

  /** Run the specified function on each element in the array. */
  void foreach_mutable(std::function<void(node &)> fn);
  void foreach(std::function<void(const node &)> fn) const;

  /** Select elements in the array matching given criteria.
   *
   * The result is a new array (with elements copy on write).
   */
  node filter(std::function<bool(const node &)> fn) const;

  /** Return number of elements in array. */
  size_t length() const;

  /** Return true iff there are no elements in the array (length==0). */
  bool is_empty() const;

  /** Remove all elements from the array. */
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


std::ostream & operator<<(std::ostream & os, const array_node & v);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_ARRAY_NODE_HPP_ */
