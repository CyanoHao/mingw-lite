#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 DWORD,
                 WINAPI,
                 GetEnvironmentVariableA,
                 _In_opt_ LPCSTR lpName,
                 _Out_opt_ LPSTR lpBuffer,
                 _In_ DWORD nSize)
  {
    d::w_str w_name;
    if (lpName && !w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return 0;
    }

    d::w_str w_buffer{d::max_path_tag{}};
    DWORD ret = GetEnvironmentVariableW(
        lpName ? w_name.c_str() : nullptr, w_buffer.data(), MAX_PATH);
    if (ret == 0)
      return 0;

    while (ret > w_buffer.size()) {
      if (!w_buffer.resize(ret)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return 0;
      }
      ret = GetEnvironmentVariableW(
          lpName ? w_name.c_str() : nullptr, w_buffer.data(), ret);
      if (ret == 0)
        return 0;
    }

    d::u_str u_buffer;
    if (!u_buffer.from_w(w_buffer.c_str(), ret)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return 0;
    }

    size_t u_size = u_buffer.size();
    if (u_size + 1 > (size_t)nSize) {
      return u_size + 1;
    }

    c::memcpy(lpBuffer, u_buffer.data(), u_size);
    lpBuffer[u_size] = 0;
    return u_size;
  }
} // namespace mingw_thunk
