#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    DWORD WINAPI fallback_WTSGetActiveConsoleSessionId();
  }
} // namespace mingw_thunk
