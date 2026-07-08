#include <thunk/_common.h>
#include <thunk/utf8-musl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 getenv,
                 const char *name)
  {
    return musl::getenv(name);
  }
} // namespace mingw_thunk
