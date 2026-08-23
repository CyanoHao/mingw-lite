#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -ve flavour: on success the process image is replaced and the call
  // does not return, which is what leaves the error paths as the only
  // testable ones.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _execve,
                 const char *file,
                 const char *const *argv,
                 const char *const *envp)
  {
    return i::proc::exec<&_wexecve, &_wexecvpe>(false, file, argv, envp);
  }
} // namespace mingw_thunk
