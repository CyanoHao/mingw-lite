#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(FILE *,
                     _wfopen,
                     const wchar_t *filename,
                     const wchar_t *mode)
  {
    if (internal::is_nt())
      return get__wfopen()(filename, mode);

    auto aname = internal::w2a(filename);
    auto amode = internal::w2a(mode);
    return fopen(aname.c_str(), amode.c_str());
  }
} // namespace mingw_thunk
