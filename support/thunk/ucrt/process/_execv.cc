#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -v flavour: _exec is _spawn with _P_OVERLAY (exec/spawnv.cpp), so
  // the wide execv pair is the target and no mode is passed.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _execv,
                 const char *file,
                 const char *const *argv)
  {
    return i::proc::exec<&_wexecve, &_wexecvpe>(false, file, argv, nullptr);
  }
} // namespace mingw_thunk
