#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 20,
                 BOOL,
                 WINAPI,
                 WriteFile,
                 _In_ HANDLE hFile,
                 _In_ LPCVOID lpBuffer,
                 _In_ DWORD nNumberOfBytesToWrite,
                 _Out_opt_ LPDWORD lpNumberOfBytesWritten,
                 _Inout_opt_ LPOVERLAPPED lpOverlapped)
  {
    DWORD mode;
    if (!GetConsoleMode(hFile, &mode))
      return get_WriteFile()(hFile,
                             lpBuffer,
                             nNumberOfBytesToWrite,
                             lpNumberOfBytesWritten,
                             lpOverlapped);

    stl::wstring w_buffer = internal::u2w((const char *)lpBuffer);
    DWORD written;
    BOOL ok =
        WriteConsoleW(hFile, w_buffer.c_str(), w_buffer.size(), &written, NULL);

    if (lpNumberOfBytesWritten) {
      if (written == w_buffer.size())
        // fast path
        *lpNumberOfBytesWritten = nNumberOfBytesToWrite;
      else {
        stl::string u_written = internal::w2u(w_buffer.c_str(), written);
        *lpNumberOfBytesWritten = u_written.size();
      }
    }

    return ok ? TRUE : FALSE;
  }
} // namespace mingw_thunk
