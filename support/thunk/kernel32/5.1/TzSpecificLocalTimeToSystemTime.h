#pragma once

#include <windows.h>

// after <windows.h>
#include <timezoneapi.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI fallback_TzSpecificLocalTimeToSystemTime(
        _In_opt_ const TIME_ZONE_INFORMATION *lpTimeZoneInformation,
        _In_ const SYSTEMTIME *lpLocalTime,
        _Out_ LPSYSTEMTIME lpUniversalTime);
  }
} // namespace mingw_thunk
