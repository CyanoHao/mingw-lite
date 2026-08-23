#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 DWORD,
                 WINAPI,
                 GetTempPathA,
                 _In_ DWORD nBufferLength,
                 _Out_ LPSTR lpBuffer)
  {
    // dry run for buffer size
    if (nBufferLength && !lpBuffer) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
    }

    d::w_str w_buffer{d::max_path_tag{}};

    DWORD ret = GetTempPathW(MAX_PATH, w_buffer.data());
    if (ret == 0)
      return 0;

    while (ret > w_buffer.size()) {
      if (!w_buffer.resize(ret)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return 0;
      }

      ret = GetTempPathW(ret, w_buffer.data());
      if (ret == 0)
        return 0;
    }

    d::u_str u_buffer;
    if (!u_buffer.from_w(w_buffer.data())) {
      SetLastError(ERROR_OUTOFMEMORY);
      return 0;
    }

    size_t u_size = u_buffer.size();
    if (u_size + 1 > nBufferLength) {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return u_size + 1;
    }

    c::memcpy(lpBuffer, u_buffer.data(), u_size);
    lpBuffer[u_size] = 0;
    return u_size;
  }
} // namespace mingw_thunk
