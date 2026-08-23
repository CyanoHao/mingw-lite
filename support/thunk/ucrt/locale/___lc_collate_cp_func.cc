#include <thunk/_common.h>

#include <windows.h>

namespace mingw_thunk
{
  // Value form (wine anchor, probe A): native C state returns 0; the
  // overlay's single collation state is UTF-8, so we self-report 65001
  // (self-consistent with setlocale -> "C.UTF-8"; divergence noted in
  // u8crt-api-set §3.4).
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 ___lc_collate_cp_func,
                 void)
  {
    return CP_UTF8;
  }
} // namespace mingw_thunk
