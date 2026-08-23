#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 20,
                 BOOL,
                 WINAPI,
                 WriteConsoleA,
                 _In_ HANDLE hConsoleOutput,
                 _In_ const VOID *lpBuffer,
                 _In_ DWORD nNumberOfCharsToWrite,
                 _Out_opt_ LPDWORD lpNumberOfCharsWritten,
                 _Reserved_ LPVOID lpReserved)
  {
    if (!lpBuffer) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    d::w_str w_buffer;
    if (!w_buffer.from_u((const char *)lpBuffer, nNumberOfCharsToWrite)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    DWORD w_written = 0;
    BOOL ok = WriteConsoleW(hConsoleOutput,
                            w_buffer.c_str(),
                            w_buffer.size(),
                            &w_written,
                            lpReserved);
    if (lpNumberOfCharsWritten) {
      if (w_written == w_buffer.size())
        // fast path
        *lpNumberOfCharsWritten = nNumberOfCharsToWrite;
      else {
        int u_written = d::u_str::size_from_w(w_buffer.c_str(), w_written);
        *lpNumberOfCharsWritten = u_written;
      }
    }
    return ok;
  }
} // namespace mingw_thunk
