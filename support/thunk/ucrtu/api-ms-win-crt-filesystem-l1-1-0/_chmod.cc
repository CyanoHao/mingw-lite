#include <thunk/_crt.h>
#include <thunk/string.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(
      api_ms_win_crt_filesystem_l1_1_0, int, _chmod, const char *path, int mode)
  {
    stl::wstring w_path = internal::u2w(path);
    return _wchmod(w_path.c_str(), mode);
  }
} // namespace mingw_thunk
