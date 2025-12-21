#pragma once

#include <minwindef.h>

#define TLS_MINIMUM_AVAILABLE 64

#ifdef __cplusplus
extern "C"
{
#endif

  __declspec(dllimport) DWORD __stdcall TlsAlloc();

#ifdef __cplusplus
}
#endif
