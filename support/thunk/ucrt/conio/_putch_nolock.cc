#include <thunk/_common.h>

#include "conio_channel.h"

namespace mingw_thunk
{
  // The nolock twin of _putch (conio/putch.cpp exposes both for the same
  // body): callers that hold the conio lock ask for the write without it
  // being taken again.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _putch_nolock,
                 int ch)
  {
    return i::conio::putch(ch);
  }
} // namespace mingw_thunk
