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
 * @file ellis_private/core/payload.hpp
 *
 * @brief payload -- refcounted payload for ellis nodes.
 *
 */

#pragma once
#ifndef ELLIS_PRIVATE_CORE_PAYLOAD_HPP_
#define ELLIS_PRIVATE_CORE_PAYLOAD_HPP_

#include <ellis/core/node.hpp>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace ellis {


namespace payload_types {
  using arr_t = std::vector<node>;
  using map_t = std::unordered_map<std::string, node>;
  using bin_t = std::vector<byte>;
  using str_t = std::string;
  using refcount_t = unsigned;
}


/** Used to store refcount and underlying container type. */
struct payload {
 /** The refcount for the underlying container. */
  payload_types::refcount_t m_refcount;
  union {
    payload_types::arr_t m_arr;
    payload_types::map_t m_map;
    payload_types::bin_t m_bin;
    payload_types::str_t m_str;
  };
};


}  /* namespace ellis */

#endif  /* ELLIS_PRIVATE_CORE_PAYLOAD_HPP_ */
