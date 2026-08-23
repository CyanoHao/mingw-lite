#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 BOOL,
                 WINAPI,
                 SetCurrentDirectoryA,
                 _In_ LPCSTR lpPathName)
  {
    if (!lpPathName) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    d::w_str w_path_name;
    if (!w_path_name.from_u(lpPathName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    return SetCurrentDirectoryW(w_path_name.c_str());
  }
} // namespace mingw_thunk
