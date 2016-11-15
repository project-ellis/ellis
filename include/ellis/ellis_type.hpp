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


/**
 * An enum containing all the Ellis data types.
 */
enum class ellis_type {
  ARRAY,
  BINARY,
  BOOL,
  DOUBLE,
  INT64,
  MAP,
  NIL,
  U8STR
};


#endif /* ELLIS_TYPE_HPP_ */
