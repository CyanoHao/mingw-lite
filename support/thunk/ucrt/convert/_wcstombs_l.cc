#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Locale argument ignored (api-set global principle 2) — delegate to
  // this layer's own wcstombs thunk (archive-internal binding).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _wcstombs_l,
                 char *dest,
                 const wchar_t *source,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return wcstombs(dest, source, count);
  }
} // namespace mingw_thunk
