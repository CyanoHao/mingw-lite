#include <thunk/_common.h>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own mblen,
  // which shares the UTF-8 engine and its shift state.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mblen_l,
                 const char *s,
                 size_t n,
                 _locale_t locale)
  {
    (void)locale;
    return mblen(s, n);
  }
} // namespace mingw_thunk
