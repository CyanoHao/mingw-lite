#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 24,
                 BOOL,
                 WINAPI,
                 CopyFileExA,
                 _In_ LPCSTR lpExistingFileName,
                 _In_ LPCSTR lpNewFileName,
                 _In_opt_ LPPROGRESS_ROUTINE lpProgressRoutine,
                 _In_opt_ LPVOID lpData,
                 _In_opt_ LPBOOL pbCancel,
                 _In_ DWORD dwCopyFlags)
  {
    if (internal::is_nt())
      return get_CopyFileExA()(lpExistingFileName,
                               lpNewFileName,
                               lpProgressRoutine,
                               lpData,
                               pbCancel,
                               dwCopyFlags);

    // Windows 9x: `CopyFileExA` is a stub.
    stl::wstring w_exist = internal::a2w(lpExistingFileName);
    stl::wstring w_new = internal::a2w(lpNewFileName);
    return CopyFileExW(w_exist.c_str(),
                       w_new.c_str(),
                       lpProgressRoutine,
                       lpData,
                       pbCancel,
                       dwCopyFlags);
  }
} // namespace mingw_thunk
