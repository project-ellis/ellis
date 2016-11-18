/*
 * @file type.h
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
enum class type {
  NIL = 0,
  BOOL = 1,
  INT64 = 2,
  DOUBLE = 3,
  U8STR = 4,
  ARRAY = 5,
  BINARY = 6,
  MAP = 7,
};


}  /* namespace ellis */

#endif /* ELLIS_TYPE_HPP_ */
