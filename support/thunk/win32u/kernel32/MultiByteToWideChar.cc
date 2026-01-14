#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/unicode.h>

#include <nocrt/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 24,
                 int,
                 WINAPI,
                 MultiByteToWideChar,
                 _In_ UINT CodePage,
                 _In_ DWORD dwFlags,
                 _In_ LPCSTR lpMultiByteStr,
                 _In_ int cbMultiByte,
                 _Out_opt_ LPWSTR lpWideCharStr,
                 _In_ int cchWideChar)
  {
    if (CodePage == CP_ACP || CodePage == CP_OEMCP) {
      CodePage = CP_UTF8;

      // unset all flags except MB_ERR_INVALID_CHARS
      dwFlags &= MB_ERR_INVALID_CHARS;
    }

    return get_MultiByteToWideChar()(CodePage,
                                     dwFlags,
                                     lpMultiByteStr,
                                     cbMultiByte,
                                     lpWideCharStr,
                                     cchWideChar);
  }
} // namespace mingw_thunk
