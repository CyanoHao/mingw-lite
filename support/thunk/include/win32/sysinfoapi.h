#pragma once

#include <minwindef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  __declspec(dllimport) DWORD __stdcall GetTickCount();
  __declspec(dllimport) unsigned long long __stdcall GetTickCount64();

#ifdef __cplusplus
}
#endif
