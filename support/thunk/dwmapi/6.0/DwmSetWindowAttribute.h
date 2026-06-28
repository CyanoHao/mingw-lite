#pragma once

#include <windows.h>

// after <windows.h>
#include <dwmapi.h>

namespace mingw_thunk
{
  namespace f
  {
    HRESULT WINAPI fallback_DwmSetWindowAttribute(_In_ HWND hwnd,
                                                  _In_ DWORD dwAttribute,
                                                  _In_ LPCVOID pvAttribute,
                                                  _In_ DWORD cbAttribute);
  }
} // namespace mingw_thunk
