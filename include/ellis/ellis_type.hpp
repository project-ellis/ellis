/*
 * @file ellis_type.h
 *
 * @brief Ellis public C header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
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
  INT,
  MAP,
  NIL,
  REAL,
  STRING
};


#endif /* ELLIS_TYPE_HPP_ */
