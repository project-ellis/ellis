/*
 * @file ellis_type.h
 *
 * @brief Ellis types C++ header.
 *
 * This is the C++ header for Ellis types.
 */

#pragma once
#ifndef ELLIS_TYPE_HPP_
#define ELLIS_TYPE_HPP_


namespace ellis {

/**
 * An enum containing all the Ellis data types.
 */
enum class ellis_type {
  ARRAY = 0,
  BINARY = 1,
  BOOL = 2,
  DOUBLE = 3,
  INT64 = 4,
  MAP = 5,
  NIL = 6,
  U8STR = 7
};


}  /* namespace ellis */

#endif /* ELLIS_TYPE_HPP_ */
