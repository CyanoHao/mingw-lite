#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOLEAN WINAPI fallback_SystemFunction036(_Out_ PVOID RandomBuffer,
                                              _In_ ULONG RandomBufferLength);
  }
} // namespace mingw_thunk
