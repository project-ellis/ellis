/*
 * @file ellis/core/system.hpp
 *
 * @brief Ellis system C++ header.
 */

#pragma once
#ifndef ELLIS_CORE_SYSTEM_HPP_
#define ELLIS_CORE_SYSTEM_HPP_

#include <sstream>


namespace ellis {


/*   ____                           _
 *  / ___|___  _ ____   _____ _ __ (_) ___ _ __   ___ ___
 * | |   / _ \| '_ \ \ / / _ \ '_ \| |/ _ \ '_ \ / __/ _ \
 * | |__| (_) | | | \ V /  __/ | | | |  __/ | | | (_|  __/
 *  \____\___/|_| |_|\_/ \___|_| |_|_|\___|_| |_|\___\___|
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
      (*::ellis::g_system_log_fn)( \
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
      (*::ellis::g_system_log_fn)( \
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
      (*::ellis::g_system_crash_fn)( \
          __FILE__, \
          __LINE__, \
          __FUNCTION__, \
          "Assert: failed expression (" #EXPR ")"); \
    } \
  } while (0)


/** Assert that the X compared to Y via OP returns true.
 *
 * Example:
 *
 *   ELLIS_ASSERT_OP(a, ==, b);
 *   ELLIS_ASSERT_OP(c, >, d);
 *   ELLIS_ASSERT_OP(d, >=, 1);
 *
 * In case of failure, the system failure function will be called, with a
 * message showing the failed condition along with the specific values of
 * X and Y, and the filename and line number.
 *
 * To use this on non-primitive types, you need to define the appropriate
 * comparison operators, as well as stream insertion (output) operators
 * for the given object type.  You'll want to make sure that the copy
 * constructor is inexpensive, since we have to make a copy of X so that
 * we can refer to it in multiple locations without side effects (this is
 * a basic issue with the macro approach.
 *
 * Always evaluated, whether in debug or release build.
 */
#define ELLIS_ASSERT_OP(X, OP, Y) \
  do { \
    /* Correctness requires making copy of X and Y. */ \
    decltype(X) copy_of_X = (X); \
    decltype(X) copy_of_Y = (decltype(X))(Y); \
    if (!(copy_of_X OP copy_of_Y)) { \
      auto xstr = ELLIS_SSTRING(copy_of_X); \
      auto ystr = ELLIS_SSTRING(copy_of_Y); \
      (*::ellis::g_system_crash_fn)( \
          __FILE__, \
          __LINE__, \
          __FUNCTION__, \
          "Assert: failed expression (" #X " " #OP " " #Y ") " \
          "with LHS = %s and RHS = %s", \
          xstr.c_str(), \
          ystr.c_str()); \
    } \
  } while (0)


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
    (*::ellis::g_system_crash_fn)( \
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
    (*::ellis::g_system_crash_fn)( \
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


/* Implementation details, referenced in macros. */
extern system_crash_fn_t g_system_crash_fn;


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
