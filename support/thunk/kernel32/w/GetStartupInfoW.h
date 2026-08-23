#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    VOID __stdcall win9x_GetStartupInfoW(_Out_ LPSTARTUPINFOW lpStartupInfo);
  }
} // namespace mingw_thunk
