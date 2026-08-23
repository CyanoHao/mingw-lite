#include "_wsystem.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, _wsystem, const wchar_t *command)
  {
    __DISPATCH_THUNK_2(
        _wsystem, i::is_nt(), &__ms__wsystem, &f::win9x__wsystem);

    return dllimport__wsystem(command);
  }

  namespace f
  {
    int win9x__wsystem(const wchar_t *command)
    {
      if (!command) {
        // availability query, pass it through
        return __ms_system(nullptr);
      }

      d::a_str a_command;
      if (!a_command.from_w(command)) {
        _set_errno(ENOMEM);
        return -1;
      }

      return __ms_system(a_command.c_str());
    }
  } // namespace f
} // namespace mingw_thunk
