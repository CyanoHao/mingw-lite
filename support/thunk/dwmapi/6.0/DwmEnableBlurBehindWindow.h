#pragma once

#include <windows.h>

// after <windows.h>
#include <dwmapi.h>

namespace mingw_thunk
{
  namespace f
  {
    HRESULT WINAPI fallback_DwmEnableBlurBehindWindow(
        _In_ HWND hWnd, _In_ const DWM_BLURBEHIND *pBlurBehind);
  }
} // namespace mingw_thunk
