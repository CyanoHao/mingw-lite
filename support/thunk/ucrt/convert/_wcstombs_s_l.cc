#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl
  wcstombs_s(size_t *conv, char *dst, size_t size, const wchar_t *src,
             size_t count);

  // Locale argument ignored — delegate to this layer's own
  // wcstombs_s thunk.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _wcstombs_s_l,
                 size_t *conv,
                 char *dst,
                 size_t size,
                 const wchar_t *src,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return wcstombs_s(conv, dst, size, src, count);
  }
} // namespace mingw_thunk
