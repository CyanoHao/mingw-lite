#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    VOID __stdcall win9x_OutputDebugStringW(_In_opt_ LPCWSTR lpOutputString);
  } // namespace f
} // namespace mingw_thunk
