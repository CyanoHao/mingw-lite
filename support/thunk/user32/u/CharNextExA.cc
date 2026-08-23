#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(user32,
                 12,
                 LPSTR,
                 WINAPI,
                 CharNextExA,
                 _In_ WORD CodePage,
                 _In_ LPCSTR lpCurrentChar,
                 _In_ DWORD dwFlags)
  {
    if (CodePage == CP_ACP || CodePage == CP_OEMCP)
      CodePage = CP_UTF8;

    return __ms_CharNextExA(CodePage, lpCurrentChar, dwFlags);
  }
} // namespace mingw_thunk
