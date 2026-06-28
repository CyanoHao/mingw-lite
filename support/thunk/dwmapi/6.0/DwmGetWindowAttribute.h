#pragma once

#include <windows.h>

// after <windows.h>
#include <dwmapi.h>

namespace mingw_thunk
{
  namespace f
  {
    HRESULT WINAPI fallback_DwmGetWindowAttribute(_In_ HWND hwnd,
                                                  _In_ DWORD dwAttribute,
                                                  _Out_ PVOID pvAttribute,
                                                  _In_ DWORD cbAttribute);
  }
} // namespace mingw_thunk
