#include "FormatMessageW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 28,
                 DWORD,
                 WINAPI,
                 FormatMessageW,
                 _In_ DWORD dwFlags,
                 _In_opt_ LPCVOID lpSource,
                 _In_ DWORD dwMessageId,
                 _In_ DWORD dwLanguageId,
                 _Out_ LPWSTR lpBuffer,
                 _In_ DWORD nSize,
                 _In_opt_ va_list *Arguments)
  {
    __DISPATCH_THUNK_2(FormatMessageW,
                       i::is_nt(),
                       &__ms_FormatMessageW,
                       &f::win9x_FormatMessageW);

    return dllimport_FormatMessageW(dwFlags,
                                    lpSource,
                                    dwMessageId,
                                    dwLanguageId,
                                    lpBuffer,
                                    nSize,
                                    Arguments);
  }

  namespace f
  {
    DWORD __stdcall win9x_FormatMessageW(_In_ DWORD dwFlags,
                                         _In_opt_ LPCVOID lpSource,
                                         _In_ DWORD dwMessageId,
                                         _In_ DWORD dwLanguageId,
                                         _Out_ LPWSTR lpBuffer,
                                         _In_ DWORD nSize,
                                         _In_opt_ va_list *Arguments)
    {
      d::a_str a_source;

      if (dwFlags & FORMAT_MESSAGE_FROM_STRING) {
        if (!lpSource) {
          SetLastError(ERROR_INVALID_PARAMETER);
          return 0;
        }

        if (!a_source.from_w((const wchar_t *)lpSource)) {
          SetLastError(ERROR_OUTOFMEMORY);
          return 0;
        }

        lpSource = a_source.data();
      }

      LPSTR a_buffer = nullptr;
      DWORD ret = __ms_FormatMessageA(dwFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                      lpSource,
                                      dwMessageId,
                                      dwLanguageId,
                                      (LPSTR)&a_buffer,
                                      0,
                                      Arguments);
      if (ret == 0)
        return 0;

      d::w_str w_buffer;
      if (!w_buffer.from_a(a_buffer, ret)) {
        LocalFree(a_buffer);
        SetLastError(ERROR_OUTOFMEMORY);
        return 0;
      }

      LocalFree(a_buffer);

      size_t w_len = w_buffer.size();

      if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
        wchar_t *allocated =
            (wchar_t *)LocalAlloc(LPTR, (w_len + 1) * sizeof(wchar_t));
        if (!allocated) {
          SetLastError(ERROR_OUTOFMEMORY);
          return 0;
        }

        c::wmemcpy(allocated, w_buffer.c_str(), w_len);
        allocated[w_len] = 0;

        *(wchar_t **)lpBuffer = allocated;
        return (DWORD)w_len;
      } else {
        if (w_len + 1 > nSize) {
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          return 0;
        }

        c::wmemcpy(lpBuffer, w_buffer.c_str(), w_len);
        lpBuffer[w_len] = 0;
        return (DWORD)w_len;
      }
    }
  } // namespace f
} // namespace mingw_thunk
