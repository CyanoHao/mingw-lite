#include <thunk/_crt.h>
#include <thunk/string.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                      int,
                      rename,
                      const char *old,
                      const char *new_)
  {
    stl::wstring w_old = internal::u2w(old);
    stl::wstring w_new = internal::u2w(new_);
    return _wrename(w_old.c_str(), w_new.c_str());
  }
} // namespace mingw_thunk
