#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // The reference mbbtype.cpp state machine with its three
  // predicates swapped to UTF-8 byte roles (plan-3 D8a; the machine
  // itself reproduces wine byte for byte).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbbtype,
                 unsigned char c,
                 int ctype)
  {
    return __crt_mbs::byte_type(c, ctype);
  }
} // namespace mingw_thunk
