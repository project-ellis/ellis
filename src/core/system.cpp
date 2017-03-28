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


#include <ellis/core/system.hpp>

#include <cfloat>
#include <cmath>
#include <ellis/core/defs.hpp>
#include <ellis/core/data_format.hpp>
#include <ellis_private/using.hpp>
#include <map>
#include <memory>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>


namespace ellis {


/**
 * A system object containing global state.
 *
 * This purpose of this object is to reduce bugs due to initialization order
 * at startup.  It may be initialized before other objects in this shared lib,
 * particularly if another library calls a function that invokes get_sys().
 * In this case, the system_singleton constructor will cause the global state
 * described here to be initialized in a consistent order.
 */
class system_singleton {
public:
  /* Data format registration map. */
  map<string, unique_ptr<const data_format>> m_data_formats;

  /* Stash of details of system crash (easy to find in a core file). */
  char          m_crash_epitaph[1024];
  const char *  m_crash_file = nullptr;
  int           m_crash_line = 0;
  const char *  m_crash_funcname = nullptr;

  /* Current system crash function. */
  system_crash_fn_t m_system_crash_fn = &default_crash_function;

  /**
   * Constructor.
   */
  system_singleton() {}
};


/**
 * Global pointer to system singleton.
 *
 * It is done this way, rather than as a static variable inside function
 * get_sys(), so that in the event of a crash one can more easily access
 * g_system_singleton (and its member variables) from the debugger.
 */
static system_singleton * g_system_singleton = nullptr;


/**
 * Create the system singleton object if it's not there yet, and return it.
 *
 * This for initialization order safety, but isn't intended to be thread safe.
 * It's supposed to be called early in process setup, before multiple threads
 * are launched.
 */
static inline system_singleton * get_sys()
{
  if (g_system_singleton == nullptr) {
    g_system_singleton = new system_singleton();
  }
  return g_system_singleton;
}


void system_add_data_format(
        unique_ptr<const data_format> fmt)
{
  get_sys()->m_data_formats.emplace(fmt->m_unique_name, std::move(fmt));
}


void system_remove_data_format(
        const char *unique_name)
{
  get_sys()->m_data_formats.erase(unique_name);
}


const data_format *system_lookup_data_format_by_unique_name(
        const char * unique_name)
{
  const auto it = get_sys()->m_data_formats.find(unique_name);
  if (it == get_sys()->m_data_formats.end()) {
    return nullptr;
  }
  return it->second.get();
}


vector<const data_format *> system_lookup_data_formats_by_extension(
        const char *extension)
{
  /* We could use a global multimap on format, but there just aren't that
   * many formats, so skip it for now. */
  vector<const data_format *> retval;
  for (const auto & it : get_sys()->m_data_formats) {
    if (it.second->m_extension == extension) {
      retval.emplace_back(it.second.get());
    }
  }
  return retval;
}


// TODO: a more reliable pairing method, not that these should ever change.
static char const * const g_sev_names[] = {
  "EMRG",
  "ALRT",
  "CRIT",
  "ERRO",
  "WARN",
  "NOTI",
  "INFO",
  "DBUG",
};


void default_system_log_function(
    log_severity sev,
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME_COARSE, &ts);
  struct tm t;
  gmtime_r(&(ts.tv_sec), &t);
  char timestr[23];
  strftime(timestr, sizeof(timestr), "%Y-%m-%d Z%T", &t);
  va_list(args);
  va_start(args, fmt);
  flockfile(stderr);
  fprintf(stderr, "%s.%03d %s/%d %s:%d ",
      timestr,
      (int)(ts.tv_nsec / 1000000),
      g_sev_names[(int)sev],
      (int)sev,
      file,
      line);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, " [%s]\n", func);
  funlockfile(stderr);
  va_end(args);
}


system_log_fn_t g_system_log_fn = &default_system_log_function;


log_severity g_system_log_prefilter = log_severity::WARN;


void set_system_log_function(system_log_fn_t fn)
{
  g_system_log_fn = fn;
}


void set_system_log_prefilter(log_severity sev)
{
  g_system_log_prefilter = sev;
}


void default_crash_function(
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...)
{
  get_sys()->m_crash_file = file;
  get_sys()->m_crash_line = line;
  get_sys()->m_crash_funcname = func;
  va_list(args);
  va_start(args, fmt);
  vsnprintf(get_sys()->m_crash_epitaph, sizeof(get_sys()->m_crash_epitaph),
      fmt, args);
  va_end(args);
  fprintf(stderr, "%s at %s:%d [%s]\n",
      get_sys()->m_crash_epitaph,
      get_sys()->m_crash_file,
      get_sys()->m_crash_line,
      get_sys()->m_crash_funcname);
  fflush(stderr);
  (*g_system_log_fn)(log_severity::CRIT, file, line, func, "%s",
      get_sys()->m_crash_epitaph);
  abort();
}


void set_system_crash_function(system_crash_fn_t fn)
{
  get_sys()->m_system_crash_fn = fn;
}


system_crash_fn_t get_system_crash_function()
{
  return get_sys()->m_system_crash_fn;
}


bool dbl_equal(double a, double b)
{
  return std::abs(a - b) <= DBL_EPSILON;
}


template <>
std::string _internal_make_str(UNUSED const std::nullptr_t &)
{
  return std::string("nullptr");
}


}  /* namespace ellis */
