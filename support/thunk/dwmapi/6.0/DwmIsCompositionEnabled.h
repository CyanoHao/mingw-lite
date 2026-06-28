#pragma once

#include <windows.h>

// after <windows.h>
#include <dwmapi.h>

namespace mingw_thunk
{
  namespace f
  {
    HRESULT WINAPI fallback_DwmIsCompositionEnabled(_Out_ BOOL *pfEnabled);
  }
} // namespace mingw_thunk
