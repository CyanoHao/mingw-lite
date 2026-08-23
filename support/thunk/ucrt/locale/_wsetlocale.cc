#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <locale.h>

namespace mingw_thunk
{
  // Mirror of the narrow setlocale (plan-2): category validated 0–5,
  // name ignored, always self-reporting the single overlay locale.
  // Native null-query returns "C" and rejects L"C.UTF-8" (probe B) —
  // self-descriptive divergence, u8crt-api-set §3.4.
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 wchar_t *,
                 __cdecl,
                 _wsetlocale,
                 int category,
                 const wchar_t *locale)
  {
    (void)locale;

    if (category < 0 || category > 5)
      return nullptr;

    static wchar_t c_utf8[] = L"C.UTF-8";
    return c_utf8;
  }
} // namespace mingw_thunk
