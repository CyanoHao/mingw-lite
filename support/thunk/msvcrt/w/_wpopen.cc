#include "_wpopen.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 FILE *,
                 __cdecl,
                 _wpopen,
                 const wchar_t *command,
                 const wchar_t *mode)
  {
    __DISPATCH_THUNK_2(_wpopen, i::is_nt(), &__ms__wpopen, &f::win9x__wpopen);

    return dllimport__wpopen(command, mode);
  }

  namespace f
  {
    FILE *win9x__wpopen(const wchar_t *command, const wchar_t *mode)
    {
      if (!command || !mode) {
        _set_errno(EINVAL);
        return nullptr;
      }

      d::a_str a_command;
      if (!a_command.from_w(command)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      d::a_str a_mode;
      if (!a_mode.from_w(mode)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      return __ms__popen(a_command.c_str(), a_mode.c_str());
    }
  } // namespace f
} // namespace mingw_thunk
