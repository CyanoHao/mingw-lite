#include <windows.h>

typeof(MultiByteToWideChar) __ms_MultiByteToWideChar;

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags,
                        const char *lpMultiByteStr, int cbMultiByte,
                        wchar_t *lpWideCharStr, int cchWideChar) {
  if (CodePage == CP_ACP || CodePage == CP_OEMCP)
    CodePage = CP_UTF8;

  return __ms_MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr,
                                  cbMultiByte, lpWideCharStr, cchWideChar);
}

#ifdef _WIN64
typeof(&MultiByteToWideChar) dllimport_MultiByteToWideChar asm(
    "__imp_MultiByteToWideChar") = &MultiByteToWideChar;
#else
typeof(&MultiByteToWideChar) dllimport_MultiByteToWideChar asm(
    "__imp__MultiByteToWideChar@24") = &MultiByteToWideChar;
#endif
