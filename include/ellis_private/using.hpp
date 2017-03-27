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
 * @file ellis_private/using.hpp
 *
 * @brief common types and headers for C++.
 *
 */

#pragma once
#ifndef ELLIS_PRIVATE_USING_HPP_
#define ELLIS_PRIVATE_USING_HPP_


#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


using std::condition_variable;
using std::deque;
using std::for_each;
using std::function;
using std::initializer_list;
using std::list;
using std::make_pair;
using std::make_shared;
using std::make_unique;
using std::map;
using std::mutex;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::shared_ptr;
using std::sort;
using std::string;
using std::unique_ptr;
using std::unique_lock;
using std::unordered_map;
using std::vector;


#endif /* ELLIS_PRIVATE_USING_HPP_ */
