#include <thunk/_common.h>

#include <locale.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 ___lc_codepage_func,
                 void)
  {
    return CP_UTF8;
  }
} // namespace mingw_thunk
