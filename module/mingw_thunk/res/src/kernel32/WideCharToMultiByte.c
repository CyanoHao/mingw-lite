#include <windows.h>

typeof(WideCharToMultiByte) __ms_WideCharToMultiByte;

int WideCharToMultiByte(UINT CodePage, DWORD dwFlags,
                        const wchar_t *lpWideCharStr, int cchWideChar,
                        char *lpMultiByteStr, int cbMultiByte,
                        const char *lpDefaultChar, BOOL *lpUsedDefaultChar) {
  if (CodePage == CP_ACP || CodePage == CP_OEMCP)
    CodePage = CP_UTF8;

  return __ms_WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar,
                                  lpMultiByteStr, cbMultiByte, lpDefaultChar,
                                  lpUsedDefaultChar);
}

#ifdef _WIN64
typeof(&WideCharToMultiByte) dllimport_WideCharToMultiByte asm(
    "__imp_WideCharToMultiByte") = &WideCharToMultiByte;
#else
typeof(&WideCharToMultiByte) dllimport_WideCharToMultiByte asm(
    "__imp__WideCharToMultiByte@32") = &WideCharToMultiByte;
#endif
