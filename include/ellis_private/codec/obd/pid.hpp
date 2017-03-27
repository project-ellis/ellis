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
 * @file codec/elm327.hpp
 *
 * @brief Ellis ELM327 codec C++ header.
 */

#pragma once
#ifndef ELLIS_CODEC_PID_HPP_
#define ELLIS_CODEC_PID_HPP_

#include <cstdint>
#include <memory>

namespace ellis {
namespace obd {


/** Gets a string describing a PID from a PID value.
 *
 * @param pid a PID
 *
 * @return a string description
 * */
const char * get_pid_string(uint16_t pid);

/** Gets a string describing a PID mode from a mode value.
 *
 * @param mode a PID mode
 *
 * @return a string description
 */
std::string get_mode_string(uint16_t mode);

/** Decodes a given value, applying the appropriate unit transformation
 * depending on the given PID.
 *
 * @param pid a PID
 * @param val an OBD II value
 *
 * @return a decoded value
 */
double decode_value(uint16_t pid, uint32_t val);


}  /* obd */
}  /* ellis */

#endif  /* ELLIS_CODEC_PID_HPP_ */
