#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  // mblen == mbtowc(nullptr, s, n) (C11 shape; plan §M5.1-3): forward
  // to this layer's own mbtowc thunk, so the UTF-8 engine and its
  // internal shift state are shared.  Native walks the ACP by locale;
  // ours is UTF-8 unconditionally (accepted divergence, plan M5.4-1).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 mblen,
                 const char *s,
                 size_t n)
  {
    return mbtowc(nullptr, s, n);
  }
} // namespace mingw_thunk
