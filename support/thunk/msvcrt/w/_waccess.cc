#include "_waccess.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errno.h>
#include <io.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      msvcrt, 0, int, __cdecl, _waccess, const wchar_t *path, int mode)
  {
    __DISPATCH_THUNK_2(
        _waccess, i::is_nt(), &__ms__waccess, &f::win9x__waccess);

    return dllimport__waccess(path, mode);
  }

  namespace f
  {
    int win9x__waccess(const wchar_t *path, int mode)
    {
      if (!path) {
        _set_errno(EINVAL);
        return -1;
      }

      d::a_str a_path;
      if (!a_path.from_w(path)) {
        _set_errno(ENOMEM);
        return -1;
      }

      return __ms__access(a_path.c_str(), mode);
    }
  } // namespace f
} // namespace mingw_thunk
