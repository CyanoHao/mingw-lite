#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 system,
                 const char *command)
  {
    if (!command) {
      // availability query, pass it through
      return _wsystem(nullptr);
    }

    d::w_str w_command;
    if (!w_command.from_u(command)) {
      _set_errno(ENOMEM);
      return -1;
    }

    return _wsystem(w_command.c_str());
  }
} // namespace mingw_thunk
