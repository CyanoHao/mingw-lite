#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    void WINAPI fallback_GetNativeSystemInfo(_Out_ LPSYSTEM_INFO lpSystemInfo);
  }
} // namespace mingw_thunk
