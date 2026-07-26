#include <windows.h>

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                   HANDLE hTemplateFile) {
  int len = MultiByteToWideChar(CP_UTF8, 0, lpFileName, -1, NULL, 0);
  if (len == 0)
    return INVALID_HANDLE_VALUE;
  wchar_t *wname = HeapAlloc(GetProcessHeap(), 0, len * sizeof(wchar_t));
  if (!wname) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return INVALID_HANDLE_VALUE;
  }
  MultiByteToWideChar(CP_UTF8, 0, lpFileName, -1, wname, len);
  HANDLE h =
      CreateFileW(wname, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                  dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
  HeapFree(GetProcessHeap(), 0, wname);
  return h;
}

#ifdef _WIN64
typeof(&CreateFileA)
    dllimport_CreateFileA asm("__imp_CreateFileA") = &CreateFileA;
#else
typeof(&CreateFileA)
    dllimport_CreateFileA asm("__imp__CreateFileA@28") = &CreateFileA;
#endif
