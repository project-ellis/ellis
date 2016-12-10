/*
 * @file ellis/core/type.h
 *
 * @brief Ellis types C++ header.
 *
 * This is the C++ header for Ellis types.
 */

#pragma once
#ifndef ELLIS_CORE_TYPE_HPP_
#define ELLIS_CORE_TYPE_HPP_


namespace ellis {

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
  MAP = 7
};


/**
 * Get a string description for a given type.
 */
const char *type_str(type t);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_TYPE_HPP_ */
