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


class system_haus {
public:
  map<string, unique_ptr<const data_format>> m_formats;
  system_haus() {}
};

static system_haus * g_system_haus = nullptr;


/**
 * Create the system if it's not there yet.
 *
 * This for initialization order safety, but isn't intended to be thread safe.
 * It's supposed to be called early in process setup, before multiple threads
 * are launched.
 */
static inline system_haus * get_sys()
{
  if (g_system_haus == nullptr) {
    g_system_haus = new system_haus();
  }
  return g_system_haus;
}


void system_add_data_format(
        unique_ptr<const data_format> fmt)
{
  get_sys()->m_formats.emplace(fmt->m_unique_name, std::move(fmt));
}


void system_remove_data_format(
        const char *unique_name)
{
  get_sys()->m_formats.erase(unique_name);
}


const data_format *system_lookup_data_format_by_unique_name(
        const char * unique_name)
{
  const auto it = get_sys()->m_formats.find(unique_name);
  if (it == get_sys()->m_formats.end()) {
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
  for (const auto & it : get_sys()->m_formats) {
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


char g_crash_epitaph[1024];
const char * g_crash_file = nullptr;
int g_crash_line = 0;
const char * g_crash_func = nullptr;


void default_crash_function(
    const char *file,
    int line,
    const char *func,
    const char *fmt, ...)
{
  g_crash_file = file;
  g_crash_line = line;
  g_crash_func = func;
  va_list(args);
  va_start(args, fmt);
  vsnprintf(g_crash_epitaph, sizeof(g_crash_epitaph), fmt, args);
  va_end(args);
  fprintf(stderr, "%s at %s:%d [%s]\n",
      g_crash_epitaph,
      g_crash_file,
      g_crash_line,
      g_crash_func);
  fflush(stderr);
  (*g_system_log_fn)(log_severity::CRIT, file, line, func, "%s",
      g_crash_epitaph);
  abort();
}


system_crash_fn_t g_system_crash_fn = &default_crash_function;


void set_system_crash_function(system_crash_fn_t fn)
{
  g_system_crash_fn = fn;
}


// TODO(jmc): make this safe w.r.t. startup order via system object.
system_crash_fn_t get_system_crash_function()
{
  return g_system_crash_fn;
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
