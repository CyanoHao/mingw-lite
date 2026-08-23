#include <thunk/_common.h>
#include "cvt_render.h"

#include <stdlib.h>

namespace mingw_thunk
{
  // Deprecated face: the buffer belongs to the caller (no size — the
  // classic contract is ~precision + 40 bytes).  Native shape == printf
  // %.*g (wine C3): "1e-05" / "1.23457e+06" / "123456" / "0.0001" /
  // "0.333333" / "0" / "-1"; two-digit exponents, trailing zeros
  // stripped.  The in-tree engine already renders that shape directly.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _gcvt,
                 double value,
                 int precision,
                 char *buffer)
  {
    if (!buffer)
      return nullptr;

    if (u8crt_convert::gcvt_core(buffer, size_t(-1) >> 1, value, precision) != 0)
      return nullptr;
    return buffer;
  }
} // namespace mingw_thunk
