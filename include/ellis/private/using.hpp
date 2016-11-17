/*
 * @file private/using.hpp
 *
 * @brief Ellis public C header.
 *
 * This is the C header for Ellis, which implements the standalone Ellis data
 * interaction routines.
 */

#pragma once
#ifndef ELLIS_PRIVATE_USING_H_
#define ELLIS_PRIVATE_USING_H_


#include <algorithm>
#include <assert.h>  // TODO: replace with ellis_system assert/fatal
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


using std::deque;
using std::for_each;
using std::function;
using std::list;
using std::make_pair;
using std::make_shared;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;


#endif /* ELLIS_PRIVATE_USING_H_ */
