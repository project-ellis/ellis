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
 * Usage:
 *
 *   set_title(ELLIS_SSTRING(pfx << "[" << dmp << "]" << getpid()).c_str());
 */
#define ELLIS_SSTRING(args) \
  ((std::ostringstream&)((std::ostringstream{}) << args)).str()


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
 *   ELLIS_LOGSTREAM(DBUG, x << ": object was: " << myobject);
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


/** Change the system log function. */
void set_system_log_function(system_log_fn_t fn);


/** Adjust the prefilter severity that gets checked before the arguments to
 * ELLIS_LOG are evaluated or forwarded to the system log function.
 *
 * You may want to automatically adjust this in response to changes to
 * the downstream system logging configuration, so as to allow for speed
 * gains while ensuring no accidental filtering of desired messages.
 * Or, for those who are lazy and less concerned about speed, it is safe
 * to set it to DBUG and forget about it.
 */
void set_system_log_prefilter(log_severity sev);


// TODO: move this stuff to private
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


void default_crash_function(
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...);

using system_crash_fn_t = decltype(&default_crash_function);

void set_system_crash_function(system_crash_fn_t fn);

extern system_crash_fn_t g_system_crash_fn;


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


#define ELLIS_ASSERT_UNREACHABLE() \
  do { \
    (*::ellis::g_system_crash_fn)( \
        __FILE__, \
        __LINE__, \
        __FUNCTION__, \
        "Assert: invalid code path reached"); \
    __builtin_unreachable(); \
  } while (0);


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


#define ELLIS_CONF(KEY)


}  /* namespace ellis */

#endif  /* ELLIS_CORE_SYSTEM_HPP_ */
