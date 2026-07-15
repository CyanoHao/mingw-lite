#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/wntcrt/find_data.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 int,
                 __cdecl,
                 _findnext64i32,
                 intptr_t handle,
                 struct _finddata64i32_t *fileinfo)
  {
    _wfinddata64i32_t w_file_info;
    int res = _wfindnext64i32(handle, &w_file_info);

    if (res == -1)
      return res;

    *fileinfo = i::w2u(w_file_info);
    return res;
  }

  // upstream added this for old msvcrt32
  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(_findnext64i32)
} // namespace mingw_thunk
