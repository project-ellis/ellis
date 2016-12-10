#include <ellis/core/system.hpp>

#include <ellis/private/using.hpp>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>


namespace ellis {


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


}  /* namespace ellis */
