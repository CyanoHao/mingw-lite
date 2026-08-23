#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    INT __stdcall win9x_GetLocaleInfoW(_In_ LCID Locale,
                                       _In_ LCTYPE LCType,
                                       _Out_opt_ LPWSTR lpLCData,
                                       _In_ int cchData);
  } // namespace f
} // namespace mingw_thunk
