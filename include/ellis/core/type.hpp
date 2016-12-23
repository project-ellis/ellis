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
  MAP = 7
};


/**
 * Get a string description for a given type.
 */
// TODO: rename to overloaded enum_name for template and macro convenience?
const char *type_str(type t);



/*  ____  _                              ____  _        _
 * / ___|| |_ _ __ ___  __ _ _ __ ___   / ___|| |_ __ _| |_ ___
 * \___ \| __| '__/ _ \/ _` | '_ ` _ \  \___ \| __/ _` | __/ _ \
 *  ___) | |_| | |  __/ (_| | | | | | |  ___) | || (_| | ||  __/
 * |____/ \__|_|  \___|\__,_|_| |_| |_| |____/ \__\__,_|\__\___|
 *
 */


enum class stream_state {
  CONTINUE,
  SUCCESS,
  ERROR,
  enum_max = ERROR
};


/**
 * Get a string version of the given enum value.
 */
const char * enum_name(stream_state x);

std::ostream & operator<<(std::ostream & os, const stream_state &s);


}  /* namespace ellis */

#endif  /* ELLIS_CORE_TYPE_HPP_ */
