#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 20,
                 BOOL,
                 WINAPI,
                 UnlockFileEx,
                 _In_ HANDLE hFile,
                 DWORD dwReserved,
                 _In_ DWORD nNumberOfBytesToLockLow,
                 _In_ DWORD nNumberOfBytesToLockHigh,
                 _Inout_ LPOVERLAPPED lpOverlapped)
  {
    if (internal::is_nt())
      return get_UnlockFileEx()(hFile,
                                dwReserved,
                                nNumberOfBytesToLockLow,
                                nNumberOfBytesToLockHigh,
                                lpOverlapped);

    return UnlockFile(
        hFile, 0, 0, nNumberOfBytesToLockLow, nNumberOfBytesToLockHigh);
  }
} // namespace mingw_thunk
