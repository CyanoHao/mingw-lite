#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <locale.h>
#include <time.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _wcsftime_l,
                 wchar_t *buf,
                 size_t count,
                 const wchar_t *format,
                 const struct tm *tm,
                 _locale_t locale)
  {
    (void)locale;
    return musl::wcsftime(buf, count, format, tm);
  }
} // namespace mingw_thunk
