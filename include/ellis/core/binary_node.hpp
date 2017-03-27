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


std::ostream & operator<<(std::ostream & os, const binary_node & v);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_BINARY_NODE_HPP_ */
