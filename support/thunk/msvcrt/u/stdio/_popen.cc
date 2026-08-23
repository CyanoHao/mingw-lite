#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/musl.h>

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

    FILE *fp = _wpopen(w_command.c_str(), w_type.c_str());
    if (fp)
      musl::console_channel_on_open(_fileno(fp));
    return fp;
  }
} // namespace mingw_thunk
