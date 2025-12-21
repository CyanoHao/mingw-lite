#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <direct.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, _wchdir, const wchar_t *dirname)
  {
    if (internal::is_nt())
      return get__wchdir()(dirname);

    stl::string a_name = internal::w2a(dirname);
    return _chdir(a_name.c_str());
  }
} // namespace mingw_thunk
