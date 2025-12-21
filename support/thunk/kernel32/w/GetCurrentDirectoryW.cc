#include "GetCurrentDirectoryW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <nocrt/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 DWORD,
                 WINAPI,
                 GetCurrentDirectoryW,
                 _In_ DWORD nBufferLength,
                 _Out_ LPWSTR lpBuffer)
  {
    if (internal::is_nt())
      return get_GetCurrentDirectoryW()(nBufferLength, lpBuffer);

    return impl::win9x_GetCurrentDirectoryW(nBufferLength, lpBuffer);
  }

  namespace impl
  {
    DWORD
    win9x_GetCurrentDirectoryW(_In_ DWORD nBufferLength, _Out_ LPWSTR lpBuffer)
    {
      char a_filename[MAX_PATH];
      DWORD a_size = kernel32_GetCurrentDirectoryA()(MAX_PATH, a_filename);
      if (a_size == 0)
        return 0;
      stl::wstring w_filename = internal::a2w(a_filename);
      if (nBufferLength >= w_filename.size() + 1) {
        wmemcpy(lpBuffer, w_filename.c_str(), w_filename.size());
        lpBuffer[w_filename.size()] = L'\0';
        return w_filename.size();
      } else {
        return w_filename.size() + 1;
      }
    }
  } // namespace impl
} // namespace mingw_thunk
