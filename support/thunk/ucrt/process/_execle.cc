#include <thunk/_common.h>

#include <stdarg.h>

#include "spawn_bridge.h"

namespace mingw_thunk
{
  // -le flavour (exec/spawnl.cpp): the varargs are captured the way
  // common_capture_argv does it -- 64 stack slots first, the heap only
  // on overflow, then the trailing envp vector for the 'e' half -- and
  // the call is handed to _wexecve. wine's own _wspawnle faults on the
  // varargs environment, which is one more reason the bridge never calls
  // the _wspawnl* pair.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _execle,
                 const char *file,
                 const char *arg0,
                 ...)
  {
    va_list ap;
    va_start(ap, arg0);

    i::proc::w_vector w_argv;
    const char *const *envp = nullptr;
    bool ok = i::proc::capture_argl(true, &ap, arg0, w_argv, envp);

    va_end(ap);

    if (!ok) {
      _set_errno(ENOMEM);
      return -1;
    }

    return i::proc::exec_w<&_wexecve, &_wexecvpe>(false, file, w_argv.c(), envp);
  }
} // namespace mingw_thunk
