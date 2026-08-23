#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -vp flavour: the native wide pair owns the %PATH% walk.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _execvp,
                 const char *file,
                 const char *const *argv)
  {
    return i::proc::exec<&_wexecve, &_wexecvpe>(true, file, argv, nullptr);
  }
} // namespace mingw_thunk
