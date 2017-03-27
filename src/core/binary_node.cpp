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


#include <ellis/core/binary_node.hpp>

#include <ellis/core/node.hpp>
#include <ellis/core/system.hpp>
#include <ellis_private/core/payload.hpp>
#include <ellis_private/using.hpp>

namespace ellis {


#define GETBIN m_node.m_pay->m_bin


binary_node::~binary_node()
{
  ELLIS_ASSERT_UNREACHABLE();
  /* This function should never be called; binary_node is a type safety wrapper
   * over node, and destruction is to be handled by node.  The user should
   * only ever see binary_node by reference, and only be able to destroy
   * a node, not a binary_node. */
}


byte& binary_node::operator[](size_t index)
{
  return GETBIN[index];
}


const byte& binary_node::operator[](size_t index) const
{
  return GETBIN[index];
}


bool binary_node::operator==(const binary_node &o) const
{
  return GETBIN == o.GETBIN;
}


void binary_node::append(const byte *srcdata, size_t len)
{
  GETBIN.insert(GETBIN.end(), srcdata, srcdata+len);
}


void binary_node::resize(size_t n)
{
  GETBIN.resize(n);
}


byte * binary_node::data()
{
  return GETBIN.data();
}


const byte * binary_node::data() const
{
  return GETBIN.data();
}


size_t binary_node::length() const
{
  return GETBIN.size();
}


bool binary_node::is_empty() const
{
  return GETBIN.empty();
}


void binary_node::clear()
{
  GETBIN.clear();
}


std::ostream & operator<<(std::ostream & os, const binary_node &v)
{
  size_t length = v.length();
  if (length > 0) {
    os << "0x";
    for (size_t i = 0; i < length; i++) {
      os << std::hex << static_cast<unsigned int>(v[i]);
    }
  }

  return os;
}


}  /* namespace ellis */
