#include <thunk/_common.h>

#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 wcstombs,
                 char *s,
                 const wchar_t *pwcs,
                 size_t n)
  {
    return wcsrtombs(s, &pwcs, n, nullptr);
  }
} // namespace mingw_thunk
