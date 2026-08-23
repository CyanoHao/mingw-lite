#include <thunk/_common.h>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own
  // mbstowcs_s, which M5 already made pure UTF-8.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbstowcs_s_l,
                 size_t *conv,
                 wchar_t *dst,
                 size_t dstsz,
                 const char *src,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return mbstowcs_s(conv, dst, dstsz, src, count);
  }
} // namespace mingw_thunk
