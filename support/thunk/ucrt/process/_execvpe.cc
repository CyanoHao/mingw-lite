#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -vpe flavour: the last of the eight.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _execvpe,
                 const char *file,
                 const char *const *argv,
                 const char *const *envp)
  {
    return i::proc::exec<&_wexecve, &_wexecvpe>(true, file, argv, envp);
  }
} // namespace mingw_thunk
