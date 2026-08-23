#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -ve flavour: a null envp is the same call as _spawnv (the identity
  // cenvarg.cpp documents), so the 'e' pair is the only target and a
  // null vector means 'inherit'.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _spawnve,
                 int mode,
                 const char *file,
                 const char *const *argv,
                 const char *const *envp)
  {
    return i::proc::spawn<&_wspawnve, &_wspawnvpe>(mode, false, file, argv, envp);
  }
} // namespace mingw_thunk
