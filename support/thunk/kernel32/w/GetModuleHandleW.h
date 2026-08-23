#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    HMODULE __stdcall win9x_GetModuleHandleW(_In_opt_ LPCWSTR lpModuleName);
  }
} // namespace mingw_thunk
