#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      msvcrt, 0, FILE *, __cdecl, _popen, const char *command, const char *type)
  {
    if (!command || !type) {
      _set_errno(EINVAL);
      return nullptr;
    }

    d::w_str w_command;
    if (!w_command.from_u(command)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    d::w_str w_type;
    if (!w_type.from_u(type)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    return _wpopen(w_command.c_str(), w_type.c_str());
  }
} // namespace mingw_thunk
