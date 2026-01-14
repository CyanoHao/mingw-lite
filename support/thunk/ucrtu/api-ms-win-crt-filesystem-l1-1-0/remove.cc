#include <thunk/_crt.h>
#include <thunk/string.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                      int,
                      remove,
                      const char *path)
  {
    stl::wstring w_path = internal::u2w(path);
    return _wremove(w_path.c_str());
  }
} // namespace mingw_thunk
