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
      get_MoveFileExA()(lpExistingFileName, lpNewFileName, dwFlags);

    // MoveFileExA is a stub
    int ok = MoveFileA(lpExistingFileName, lpNewFileName);
    if (!ok &&
        (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))) {
      bool replace = dwFlags & MOVEFILE_REPLACE_EXISTING;
      ok = CopyFileA(lpExistingFileName, lpNewFileName, !replace);
      if (ok)
        DeleteFileA(lpExistingFileName);
    }
    return ok;
  }
} // namespace mingw_thunk
