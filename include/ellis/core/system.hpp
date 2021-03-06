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
 * @file ellis/core/system.hpp
 *
 * @brief Ellis system C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_SYSTEM_HPP_
#define ELLIS_CORE_SYSTEM_HPP_

#include <cstring>
//#include <functional>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>
#include <ellis/core/defs.hpp>


namespace ellis {


/*   ____                           _
 *  / ___|___  _ ____   _____ _ __ (_) ___ _ __   ___ ___
 * | |   / _ \| '_ \ \ / / _ \ '_ \| |/ _ \ '_ \ / __/ _ \
 * | |__| (_) | | | \ V /  __/ | | | |  __/ | | | (_|  __/
 *  \____\___/|_| |_|\_/ \___|_| |_|_|\___|_| |_|\___\___|
 *
 * (for use throughout system, but also used in within this header).
 *
 */


/** Stream to string helper.
 *
 * Creates a temporary string stream, sends macro arguments to that stream,
 * and gets the string result.
 *
 * Example:
 *
 *   // This function becomes a one-liner...
 *   string myfunc() {
 *     return ELLIS_SSTRING(pfx << "[" << dmp << "]" << getpid());
 *   }
 *
 * Warning:
 *
 *   If you are using the result with a function that takes a const char *,
 *   you can safely pass the c_str() output from the resulting string, if the
 *   value will be used only during the function call and will not need to
 *   live on after the function call terminates.  Thus the following is safe:
 *
 *     somefunc(ELLIS_SSTRING(left << x << right).c_str());
 *
 *   The following is also safe:
 *
 *     auto s = ELLIS_SSTRING(left << x << right);
 *     somefunc(s.c_str());
 *
 *   But the following is NOT SAFE:
 *
 *     const char *p = ELLIS_SSTRING(left << x << right).c_str();  // trouble
 *     somefunc(p);
 *
 *   The reason the latter is not safe is that the string has already been
 *   destroyed by the time p is used, and the memory is undefined.
 */
#define ELLIS_SSTRING(args) \
  ((::std::ostringstream&)((::std::ostringstream{}) << args)).str()



/*  ____            _     _             _   _
 * |  _ \ ___  __ _(_)___| |_ _ __ __ _| |_(_) ___  _ __
 * | |_) / _ \/ _` | / __| __| '__/ _` | __| |/ _ \| '_ \
 * |  _ <  __/ (_| | \__ \ |_| | | (_| | |_| | (_) | | | |
 * |_| \_\___|\__, |_|___/\__|_|  \__,_|\__|_|\___/|_| |_|
 *            |___/
 *
 * (registration of plugins, codecs, etc).
 */


// forward declaration
struct data_format;


/**
 * Register a new data format.
 *
 * See data_format.h for more information on data formats.
 *
 * Ownership is transferred.  No further modifications are allowed.
 * There must not be a registered data format of the same unique name.
 *
 * Not threadsafe.
 */
void system_add_data_format(
    std::unique_ptr<const data_format> fmt);


/**
 * Unregister a data format.
 *
 * See data_format.h for more information on data formats.
 *
 * There must be a previously registered data format with the given name.
 *
 * Not threadsafe.
 */
void system_remove_data_format(
    const char *unique_name);


/**
 * Lookup a data format via its unique name.
 *
 * @return : non-owning pointer to the const data_format record uniquely
 *   identified by the name, or nullptr if no such entry is registered.
 *
 * No modifications to the data_format record are allowed.
 *
 * Not threadsafe.
 */
const data_format *system_lookup_data_format_by_unique_name(
    const char *unique_name);


/**
 * Find the list of data formats apropos the given filename extension.
 *
 * @return : a vector of non-owning pointers to const data_format records,
 *   possibly empty.
 *
 * No modifications to the records are allowed.
 *
 * Not threadsafe.
 */
std::vector<const data_format *> system_lookup_data_formats_by_extension(
    const char *extension);



/*  _                      _               _   _             _
 * | |    ___   __ _  __ _(_)_ __   __ _  | | | | ___   ___ | | _____
 * | |   / _ \ / _` |/ _` | | '_ \ / _` | | |_| |/ _ \ / _ \| |/ / __|
 * | |__| (_) | (_| | (_| | | | | | (_| | |  _  | (_) | (_) |   <\__ \
 * |_____\___/ \__, |\__, |_|_| |_|\__, | |_| |_|\___/ \___/|_|\_\___/
 *             |___/ |___/         |___/
 */


/** Use this macro to log.  It has printf semantics with a severity,
 * and for the sake of performance will only evaluate its arguments and
 * forward to the system logging function if the severity prefilter is
 * breached.
 *
 * Example:
 *
 *   ELLIS_LOG(DBUG, "unrecognized record %s", rec.c_str());
 *
 * In this macro, the names are intentionally fully qualified.
 */
#define ELLIS_LOG(LVL, ...) \
  do { \
    if ((int)::ellis::log_severity::LVL \
        <= (int)::ellis::g_system_log_prefilter) \
    { \
      (*::ellis::get_system_log_function())( \
          ::ellis::log_severity::LVL, \
          __FILE__, \
          __LINE__, \
          __FUNCTION__, \
          __VA_ARGS__); \
    } \
  } while(0)


/** This is like ELLIS_LOG but with streaming semantics.
 *
 * Example:
 *
 *   ELLIS_LOGSTREAM(DBUG, "x was: " << x << ", obj was: " << obj);
 */
#define ELLIS_LOGSTREAM(LVL, ...) \
  do { \
    if ((int)::ellis::log_severity::LVL \
        <= (int)::ellis::g_system_log_prefilter) \
    { \
      (*::ellis::get_system_log_function())( \
          ::ellis::log_severity::LVL, \
          __FILE__, \
          __LINE__, \
          __FUNCTION__, \
          "%s", \
          ELLIS_SSTRING(__VA_ARGS__).c_str()); \
    } \
  } while(0)


/** The severity levels used in the ELLIS_LOG macro.
 *
 * They should not be changed.
 */
enum class log_severity {
  EMRG = 0,
  ALRT = 1,
  CRIT = 2,
  ERRO = 3,
  WARN = 4,
  NOTI = 5,
  INFO = 6,
  DBUG = 7,
};


/** The default logging in ELLIS simply dumps to stderr. */
void default_system_log_function(
    log_severity sev,
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...);


/** The type of logging functions that ELLIS accepts. */
using system_log_fn_t = decltype(&default_system_log_function);


/** Replace the system log function.
 *
 * You can use this to redirect ellis log output into your the logging
 * system that your software uses.
 */
void set_system_log_function(system_log_fn_t fn);


/**
 * Return the current system log function.
 */
system_log_fn_t get_system_log_function();


/** Adjust the prefilter severity that gets checked before the arguments to
 * ELLIS_LOG are evaluated or forwarded to the system log function.
 *
 * You may wish to automatically adjust this in response to changes to
 * the downstream system logging configuration, so as to allow for speed
 * gains while ensuring no accidental filtering of desired messages.
 *
 * Or, for those who are lazy and less concerned about speed, it is safe
 * to set it to DBUG and let the system log function that you specified
 * to set_system_log_function do the filtering.  This is simpler but it
 * means that arguments to ELLIS_LOG would be evaluated, and the function
 * called, in all cases, which could result in lower performance.
 */
void set_system_log_prefilter(log_severity sev);


/* Implementation details, referenced in macros. */
extern system_log_fn_t g_system_log_fn;
/* Implementation detail; not placed inside global system object because
 * we do not want to invoke get_sys() every time we check the prefilter. */
extern log_severity g_system_log_prefilter;


/*  _____     _        _                       _
 * |  ___|_ _| |_ __ _| |___    __ _ _ __   __| |
 * | |_ / _` | __/ _` | / __|  / _` | '_ \ / _` |
 * |  _| (_| | || (_| | \__ \ | (_| | | | | (_| |
 * |_|  \__,_|\__\__,_|_|___/  \__,_|_| |_|\__,_|
 *
 *     _                      _
 *    / \   ___ ___  ___ _ __| |_ ___
 *   / _ \ / __/ __|/ _ \ '__| __/ __|
 *  / ___ \\__ \__ \  __/ |  | |_\__ \
 * /_/   \_\___/___/\___|_|   \__|___/
 *
 */


/** Assert that EXPR is true.
 *
 * In case of failure, the system failure function will be called, with a
 * message describing the assert condition, filename and line number.
 *
 * Always evaluated, whether in debug or release build.
 */
#define ELLIS_ASSERT(EXPR) \
do { \
  if (EXPR) { \
    /* Empty, but catches accidental assignment (i.e. a=b) in EXPR. */ \
  } else { \
    (*::ellis::get_system_crash_function())( \
        __FILE__, \
        __LINE__, \
        __FUNCTION__, \
        "Assert: failed expression (" #EXPR ")"); \
    ELLIS_UNREACHABLE_HINT() \
  } \
} while (0)


/** Assert that the X compared to Y via OP returns true.
 *
 * Examples:
 *
 *   ELLIS_ASSERT_LT(a, b);
 *   ELLIS_ASSERT_LTE(a, b);
 *   ELLIS_ASSERT_EQ(a, b);
 *   ELLIS_ASSERT_NEQ(a, b);
 *   ELLIS_ASSERT_GT(a, b);
 *   ELLIS_ASSERT_GTE(a, b);
 *   ELLIS_ASSERT_DBL_EQ(a, b);
 *   ELLIS_ASSERT_NULL(p);
 *   ELLIS_ASSERT_NOT_NULL(p);
 *
 * In case of failure, the system failure function will be called, with a
 * message showing the failed condition along with the specific values of
 * X and Y, and the filename and line number.
 *
 * To use this on non-primitive types, you need to define the appropriate
 * comparison operators, as well as stream insertion (output) operators
 * for the given object type.
 *
 * Always evaluated, whether in debug or release build.
 */
#define ELLIS_ASSERT_LT(x, y) ::ellis::_assert_lt(#x "<" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_LTE(x, y) ::ellis::_assert_lte(#x "<=" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_EQ(x, y) ::ellis::_assert_eq(#x "==" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_NEQ(x, y) ::ellis::_assert_neq(#x "!=" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_GT(x, y) ::ellis::_assert_gt(#x ">" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_GTE(x, y) ::ellis::_assert_gte(#x ">=" #y, x, y, __FILE__, __LINE__, __FUNCTION__)
#define ELLIS_ASSERT_MEM_EQ(x, y, count) ::ellis::_assert_mem_eq(x, y, count, __FILE__, __LINE__, __FUNCTION__)

#define ELLIS_ASSERT_TRUE(x) ELLIS_ASSERT_EQ(x, true)
#define ELLIS_ASSERT_FALSE(x) ELLIS_ASSERT_EQ(x, false)
#define ELLIS_ASSERT_DBL_EQ(x, y) ELLIS_ASSERT_TRUE(dbl_equal(x, y))
#define ELLIS_ASSERT_NULL(x) ELLIS_ASSERT_EQ(x, nullptr)
#define ELLIS_ASSERT_NOT_NULL(x) ELLIS_ASSERT_NEQ(x, nullptr)


/* Internal macro--leave alone.  We need unreachability hints to tell the
 * compilers that we know something is unreachable and it's not an error, but
 * in tests of these macros, we may intentionally not exit after supposedly
 * unreachable code, though __builtin_unreachable normally results in
 * undefined behavior for code that follows it, which would result in
 * unpredictable tests, so such tests would need to disable the unreachability
 * hint. */
#ifndef ELLIS_DISABLE_UNREACHABLE_HINT
#define ELLIS_UNREACHABLE_HINT() __builtin_unreachable();
#else
#define ELLIS_UNREACHABLE_HINT()
#endif


/** Assert that this line should never be reached.
 *
 * In case of failure, the system failure function will be called, with a
 * message explaining the problem and noting filename and line number.
 *
 * Always evaluated, whether in debug or release build.
 */
#define ELLIS_ASSERT_UNREACHABLE() \
  do { \
    (*::ellis::get_system_crash_function())( \
        __FILE__, \
        __LINE__, \
        __FUNCTION__, \
        "Assert: invalid code path reached"); \
    ELLIS_UNREACHABLE_HINT() \
  } while (0)


/** Call the ellis system crash function with printf message semantics,
 * annotating with file/line.
 */
#define ELLIS_CRASH(...) \
  do { \
    (*::ellis::get_system_crash_function())( \
        __FILE__, \
        __LINE__, \
        __FUNCTION__, \
        __VA_ARGS__); \
    ELLIS_UNREACHABLE_HINT() \
  } while (0)


/** The default logging in ELLIS prints to stderr, logs, and calls abort()
 * which generates a core dump.
 */
void default_crash_function(
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...);


/** The type of crash functions that ELLIS accepts. */
using system_crash_fn_t = decltype(&default_crash_function);


/** Replace the system crash function, which is called when ellis encounters
 * a fatal error.
 */
void set_system_crash_function(system_crash_fn_t fn);


/**
 * Return the current system crash function.
 */
system_crash_fn_t get_system_crash_function();


bool dbl_equal(double a, double b);


/** Internal function--leave alone. Used for ensuring that nullptr_t can be
 * conveniently streamed for assertions. See here for more details:
 *
 * http://cplusplus.github.io/LWG/lwg-defects.html#2221
 */
template <typename T>
std::string _internal_make_str(const T &t)
{
  return ELLIS_SSTRING(t);
}
template <>
std::string _internal_make_str(const std::nullptr_t &);


/** Internal macro--leave alone. Used to generate assertion functions for a
 * given operator.
 */
#define _INTERNAL_GEN_ASSERTS(op, name) \
template <typename T, typename U> \
static inline void _assert_##name( \
    const char *expr, \
    const T &x, \
    const U &y, \
    const char *file, \
    int line, \
    const char *function) \
{ \
  if (x op static_cast<decltype(x)>(y)) { \
    return; \
  } \
  \
  const std::string &msg = \
    ELLIS_SSTRING("Assert: failed expression (" << expr << ") " \
    "with LHS = %s and RHS = %s"); \
  (*ellis::get_system_crash_function())( \
      file, \
      line, \
      function, \
      msg.c_str(), \
      _internal_make_str(x).c_str(), \
      _internal_make_str(y).c_str()); \
  ELLIS_UNREACHABLE_HINT() \
}

_INTERNAL_GEN_ASSERTS(<, lt)
_INTERNAL_GEN_ASSERTS(<=, lte)
_INTERNAL_GEN_ASSERTS(==, eq)
_INTERNAL_GEN_ASSERTS(!=, neq)
_INTERNAL_GEN_ASSERTS(>, gt)
_INTERNAL_GEN_ASSERTS(>=, gte)


static inline void _stream_mem_block(
    std::ostringstream &ss,
    const byte *mem,
    size_t count)
{
  const byte *end = mem + count;
  for (const byte *p = mem; p < end; p++) {
    ss << "0x"
       << std::hex
       << std::setfill('0')
       << std::setw(2)
       << static_cast<unsigned int>(*p);
    if (p < end-1) {
      ss <<  ' ';
    }
  }
}


static inline void _assert_mem_eq(
    const byte *x,
    const byte *y,
    size_t count,
    const char *file,
    int line,
    const char *function)
{
  if (std::memcmp(x, y, count) == 0) {
    return;
  }

  /* Find the first differing point so we can print it out. */
  size_t diff;
  for (diff = 0; diff < count; diff++) {
    if (x[diff] != y[diff]) {
      break;
    }
  }
  ELLIS_ASSERT_NEQ(diff, count);

  const char *lhs_prefix = "LHS: { ";
  const char *rhs_prefix = "RHS: { ";

  std::ostringstream ss;
  ss << "Assert: memory blocks are not equal:\n";
  ss << lhs_prefix;
  _stream_mem_block(ss, x, count);
  ss << " }\n";

  ss << rhs_prefix;
  _stream_mem_block(ss, y, count);
  ss << " }\n";

  static size_t prefix_len = std::max(
      std::strlen(lhs_prefix),
      std::strlen(rhs_prefix));
  for (size_t i = 0; i < prefix_len; i++) {
    ss << ' ';
  }
  for (size_t i = 0; i < diff; i++) {
    ss << "     ";
  }
  ss << "  ^\n";

  (*ellis::get_system_crash_function())(
      file,
      line,
      function,
      ss.str().c_str(),
      nullptr,
      nullptr);
  ELLIS_UNREACHABLE_HINT();
}



/*  ____            _                               _     _
 * / ___| _   _ ___| |_ ___ _ __ ___      __      _(_) __| | ___
 * \___ \| | | / __| __/ _ \ '_ ` _ \ ____\ \ /\ / / |/ _` |/ _ \
 *  ___) | |_| \__ \ ||  __/ | | | | |_____\ V  V /| | (_| |  __/
 * |____/ \__, |___/\__\___|_| |_| |_|      \_/\_/ |_|\__,_|\___|
 *        |___/
 *   ____             __ _
 *  / ___|___  _ __  / _(_) __ _
 * | |   / _ \| '_ \| |_| |/ _` |
 * | |__| (_) | | | |  _| | (_| |
 *  \____\___/|_| |_|_| |_|\__, |
 *                         |___/
 */


// Big TBD; config like an area where we might need to interoperate with
// a larger system.
#define ELLIS_GETCONF(KEY)


}  /* namespace ellis */

#endif  /* ELLIS_CORE_SYSTEM_HPP_ */
