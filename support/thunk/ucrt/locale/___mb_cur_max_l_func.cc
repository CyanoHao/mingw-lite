#include <thunk/_common.h>

#include <locale.h>

namespace mingw_thunk
{
  // Delegates to the un-suffixed ___mb_cur_max_func (the only true state
  // is C.UTF-8, so the _l variant ignores its locale argument).  Native
  // C state returns 1; ours self-reports 4 — same divergence as
  // ___mb_cur_max_func (plan-2 M-note, u8crt-api-set §3.4).
  extern "C" int __cdecl ___mb_cur_max_func(void);

  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 ___mb_cur_max_l_func,
                 _locale_t locale)
  {
    (void)locale;
    return ___mb_cur_max_func();
  }
} // namespace mingw_thunk
