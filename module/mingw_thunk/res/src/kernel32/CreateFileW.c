#ifndef _WIN64

#include <windows.h>

typeof(CreateFileW) __ms_CreateFileW;
typeof(CreateFileA) __ms_CreateFileA;
typeof(WideCharToMultiByte) __ms_WideCharToMultiByte;

HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                   HANDLE hTemplateFile) {
  if (GetVersion() < 0x80000000)
    return __ms_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                            lpSecurityAttributes, dwCreationDisposition,
                            dwFlagsAndAttributes, hTemplateFile);

  int len =
      __ms_WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, NULL, 0, NULL, NULL);
  if (len == 0)
    return INVALID_HANDLE_VALUE;
  char *name = HeapAlloc(GetProcessHeap(), 0, len);
  if (!name) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return INVALID_HANDLE_VALUE;
  }
  __ms_WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, name, len, NULL, NULL);
  HANDLE h = __ms_CreateFileA(name, dwDesiredAccess, dwShareMode,
                              lpSecurityAttributes, dwCreationDisposition,
                              dwFlagsAndAttributes, hTemplateFile);
  HeapFree(GetProcessHeap(), 0, name);
  return h;
}

typeof(&CreateFileW)
    dllimport_CreateFileW asm("__imp__CreateFileW@28") = &CreateFileW;

#endif
