#include <thunk/_common.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -vp flavour: the %PATH% walk and its EACCES/ENOENT ladder live in
  // the native _wspawnvpe (exec/spawnvp.cpp), so the entry is pure
  // delegation.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _spawnvp,
                 int mode,
                 const char *file,
                 const char *const *argv)
  {
    return i::proc::spawn<&_wspawnve, &_wspawnvpe>(mode, true, file, argv, nullptr);
  }
} // namespace mingw_thunk
