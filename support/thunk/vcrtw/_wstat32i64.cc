#include <thunk/_crt.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <sys/stat.h>
#include <wchar.h>

#undef _wstat32i64

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(int,
                     _wstat32i64,
                     const wchar_t *path,
                     struct _stat32i64 *buffer)
  {
    if (internal::is_nt())
      return crt__wstat32i64()(path, buffer);

    stl::string a_path = internal::w2a(path);
    return _stat32i64(a_path.c_str(), buffer);
  }
} // namespace mingw_thunk
