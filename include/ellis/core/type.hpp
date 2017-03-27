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
 * @file ellis/core/type.hpp
 *
 * @brief Ellis types C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_TYPE_HPP_
#define ELLIS_CORE_TYPE_HPP_

#include <memory>
#include <ostream>

namespace ellis {


/*  ____        _                     _____
 * |  _ \  __ _| |_ _   _ _ __ ___   |_   _|   _ _ __   ___
 * | | | |/ _` | __| | | | '_ ` _ \    | || | | | '_ \ / _ \
 * | |_| | (_| | |_| |_| | | | | | |   | || |_| | |_) |  __/
 * |____/ \__,_|\__|\__,_|_| |_| |_|   |_| \__, | .__/ \___|
 *                                         |___/|_|
 */


/**
 * An enum containing all the Ellis data types.
 */
enum class type {
  NIL = 0,
  BOOL = 1,
  INT64 = 2,
  DOUBLE = 3,
  U8STR = 4,
  ARRAY = 5,
  BINARY = 6,
  MAP = 7,
  refcntd = U8STR,
  enum_max = MAP
};


/**
 * Get a string description for a given type.
 */
const char *type_str(type t);



/*  ____  _                              ____  _        _
 * / ___|| |_ _ __ ___  __ _ _ __ ___   / ___|| |_ __ _| |_ ___
 * \___ \| __| '__/ _ \/ _` | '_ ` _ \  \___ \| __/ _` | __/ _ \
 *  ___) | |_| | |  __/ (_| | | | | | |  ___) | || (_| | ||  __/
 * |____/ \__|_|  \___|\__,_|_| |_| |_| |____/ \__\__,_|\__\___|
 *
 */


/**
 * An enum differentiating the various states of production/consumption.
 */
enum class stream_state {
  CONTINUE,   //<<< Working intermediate state--more data needed--keep going.
  SUCCESS,    //<<< An entity has completed (e.g. a value deserialized).
  ERROR,      //<<< Production or consumption interrupted due to error.
  enum_max = ERROR
};


/**
 * Get a string version of the given enum value.
 */
const char * enum_name(stream_state x);


/**
 * Iostream insertion operator for stream_state.
 *
 * This is here so that for example an ELLIS_ASSERT_EQ on two stream_state
 * values can show the pretty names rather than numbers when the two states
 * do not match.
 */
std::ostream & operator<<(std::ostream & os, const stream_state &s);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_TYPE_HPP_ */
