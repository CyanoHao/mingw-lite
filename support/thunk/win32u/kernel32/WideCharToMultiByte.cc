#include <thunk/_common.h>
#include <thunk/os.h>
#include <thunk/unicode.h>

#include <nocrt/wchar.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 32,
                 int,
                 WINAPI,
                 WideCharToMultiByte,
                 _In_ UINT CodePage,
                 _In_ DWORD dwFlags,
                 _In_ LPCWCH lpWideCharStr,
                 _In_ int cchWideChar,
                 _Out_opt_ LPSTR lpMultiByteStr,
                 _In_ int cbMultiByte,
                 _In_opt_ LPCCH lpDefaultChar,
                 _In_opt_ LPBOOL lpUsedDefaultChar)
  {
    if (CodePage == CP_ACP || CodePage == CP_OEMCP) {
      CodePage = CP_UTF8;

      lpDefaultChar = nullptr;
      lpUsedDefaultChar = nullptr;

      // unset all flags except WC_ERR_INVALID_CHARS
      dwFlags &= WC_ERR_INVALID_CHARS;
    }

    return get_WideCharToMultiByte()(CodePage,
                                     dwFlags,
                                     lpWideCharStr,
                                     cchWideChar,
                                     lpMultiByteStr,
                                     cbMultiByte,
                                     lpDefaultChar,
                                     lpUsedDefaultChar);
  }
} // namespace mingw_thunk
