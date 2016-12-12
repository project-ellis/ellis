/*
 * @file ellis/core/defs.hpp
 *
 * @brief Ellis TBD C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_DEFS_HPP_
#define ELLIS_CORE_DEFS_HPP_

#include <climits>

namespace ellis {


using byte = unsigned char;
const byte BYTE_MIN = 0;
const byte BYTE_MAX = UCHAR_MAX;


#define UNUSED __attribute__((unused))


}  /* namespace ellis */

#endif  /* ELLIS_CORE_DEFS_HPP_ */
