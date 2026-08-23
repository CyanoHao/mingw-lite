#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -vpe flavour: the wide vector carries the caller's environment and
  // the search ladder stays native.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _spawnvpe,
                 int mode,
                 const char *file,
                 const char *const *argv,
                 const char *const *envp)
  {
    return i::proc::spawn<&_wspawnve, &_wspawnvpe>(mode, true, file, argv, envp);
  }
} // namespace mingw_thunk
