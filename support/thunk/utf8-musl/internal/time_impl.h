#pragma once

/* musl src/time/time_impl.h — the subset consumed by the M9 engine
 * ports (plan-2 §4.3): __tm_to_secs feeds the strftime %s specifier;
 * __strftime_fmt_1 is shared by the strftime and wcsftime engines.
 * The Windows struct tm has no __tm_gmtoff/__tm_tzname — the %z/%Z/%s
 * specifiers read the tz state (thunk/u8crt/musl.h, tz namespace)
 * instead. */

#include <stddef.h>
#include <time.h>

namespace mingw_thunk
{
  namespace musl
  {
    int __month_to_secs(int month, int is_leap);
    long long __year_to_secs(long long year, int *is_leap);
    long long __tm_to_secs(const struct tm *tm);
    const char *__strftime_fmt_1(char (*s)[100], size_t *l, int f,
                                 const struct tm *tm, int pad);
  } // namespace musl
} // namespace mingw_thunk
