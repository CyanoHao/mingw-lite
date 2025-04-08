#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 BOOL,
                 WINAPI,
                 MoveFileExW,
                 _In_ LPCWSTR lpExistingFileName,
                 _In_opt_ LPCWSTR lpNewFileName,
                 _In_ DWORD dwFlags)
  {
    if (internal::is_nt())
      get_MoveFileExW()(lpExistingFileName, lpNewFileName, dwFlags);

    auto aexist = internal::narrow(lpExistingFileName);
    if (lpNewFileName) {
      auto anew = internal::narrow(lpNewFileName);
      return MoveFileExA(aexist.c_str(), anew.c_str(), dwFlags);
    } else {
      return MoveFileExA(aexist.c_str(), nullptr, dwFlags);
    }
  }
} // namespace mingw_thunk
