#pragma once

#include <minwindef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  __declspec(dllimport) HANDLE __stdcall GetProcessHeap();
  __declspec(dllimport) void *__stdcall
  HeapAlloc(HANDLE hHeap, DWORD dwFlags, size_t dwBytes);
  __declspec(dllimport) BOOL __stdcall
  HeapFree(HANDLE hHeap, DWORD dwFlags, void *lpMem);
  __declspec(dllimport) void *__stdcall
  HeapReAlloc(HANDLE hHeap, DWORD dwFlags, void *lpMem, size_t dwBytes);

#ifdef __cplusplus
}
#endif
