#include <thunk/_common.h>

#include <wchar.h>

namespace mingw_thunk
{
  // D12: six per-category name slots, all self-reporting the single
  // overlay locale.  Native C state returns six NULL slots (probe A) —
  // self-descriptive divergence, u8crt-api-set §3.4.
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 wchar_t **,
                 __cdecl,
                 ___lc_locale_name_func,
                 void)
  {
    static wchar_t c_utf8[] = L"C.UTF-8";
    static wchar_t *slots[6] = {c_utf8, c_utf8, c_utf8, c_utf8, c_utf8,
                                c_utf8};
    return slots;
  }
} // namespace mingw_thunk
