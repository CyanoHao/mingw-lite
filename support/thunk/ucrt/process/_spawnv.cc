#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -v flavour (exec/spawnv.cpp): the caller's vector is converted and
  // the whole call handed to _wspawnve; the mode, the extension probing
  // and the inherited-handle block all stay native.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _spawnv,
                 int mode,
                 const char *file,
                 const char *const *argv)
  {
    return i::proc::spawn<&_wspawnve, &_wspawnvpe>(mode, false, file, argv, nullptr);
  }
} // namespace mingw_thunk
