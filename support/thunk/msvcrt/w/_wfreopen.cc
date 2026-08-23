#include "_wfreopen.h"

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
                 _wfreopen,
                 const wchar_t *path,
                 const wchar_t *mode,
                 FILE *stream)
  {
    __DISPATCH_THUNK_2(
        _wfreopen, i::is_nt(), &__ms__wfreopen, &f::win9x__wfreopen);

    return dllimport__wfreopen(path, mode, stream);
  }

  namespace f
  {
    FILE *
    win9x__wfreopen(const wchar_t *path, const wchar_t *mode, FILE *stream)
    {
      if (!path || !mode) {
        _set_errno(EINVAL);
        return nullptr;
      }

      d::a_str a_path;
      if (!a_path.from_w(path)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      d::a_str a_mode;
      if (!a_mode.from_w(mode)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      return __ms_freopen(a_path.c_str(), a_mode.c_str(), stream);
    }
  } // namespace f
} // namespace mingw_thunk
