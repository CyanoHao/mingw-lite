#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/ucrt.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 fwrite,
                 const void *ptr,
                 size_t size,
                 size_t count,
                 FILE *stream)
  {
    return ucrt::fwrite(ptr, size, count, stream);
  }
} // namespace mingw_thunk
