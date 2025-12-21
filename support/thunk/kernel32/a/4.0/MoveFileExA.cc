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
                 MoveFileExA,
                 _In_ LPCSTR lpExistingFileName,
                 _In_opt_ LPCSTR lpNewFileName,
                 _In_ DWORD dwFlags)
  {
    if (internal::is_nt())
      return get_MoveFileExA()(lpExistingFileName, lpNewFileName, dwFlags);

    // Windows 9x: MoveFileExA is a stub
    stl::wstring w_exist = internal::a2w(lpExistingFileName);
    stl::wstring w_new;
    if (lpNewFileName)
      w_new = internal::a2w(lpNewFileName);
    return MoveFileExW(
        w_exist.c_str(), lpNewFileName ? w_new.c_str() : nullptr, dwFlags);
  }
} // namespace mingw_thunk
