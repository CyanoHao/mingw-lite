#include <thunk/_crt.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <sys/stat.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(int, _wstat32, const wchar_t *path, struct _stat32 *buffer)
  {
    if (internal::is_nt())
      return crt__wstat32()(path, buffer);

    stl::string a_path = internal::w2a(path);
    return _stat32(a_path.c_str(), buffer);
  }
} // namespace mingw_thunk
