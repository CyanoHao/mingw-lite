#include <thunk/_common.h>

#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 mbstowcs,
                 wchar_t *pwcs,
                 const char *s,
                 size_t n)
  {
    return mbsrtowcs(pwcs, &s, n, nullptr);
  }
} // namespace mingw_thunk
