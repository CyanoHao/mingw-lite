#include "TzSpecificLocalTimeToSystemTime.h"

#include <thunk/_common.h>

#include <timezoneapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 BOOL,
                 WINAPI,
                 TzSpecificLocalTimeToSystemTime,
                 _In_opt_ const TIME_ZONE_INFORMATION *lpTimeZoneInformation,
                 _In_ const SYSTEMTIME *lpLocalTime,
                 _Out_ LPSYSTEMTIME lpUniversalTime)
  {
    __DISPATCH_THUNK_2(TzSpecificLocalTimeToSystemTime,
                       const auto pfn =
                           try_get_TzSpecificLocalTimeToSystemTime(),
                       pfn,
                       &f::fallback_TzSpecificLocalTimeToSystemTime);

    return dllimport_TzSpecificLocalTimeToSystemTime(
        lpTimeZoneInformation, lpLocalTime, lpUniversalTime);
  }

  namespace f
  {
    BOOL WINAPI fallback_TzSpecificLocalTimeToSystemTime(
        _In_opt_ const TIME_ZONE_INFORMATION *lpTimeZoneInformation,
        _In_ const SYSTEMTIME *lpLocalTime,
        _Out_ LPSYSTEMTIME lpUniversalTime)
    {
      const TIME_ZONE_INFORMATION *tzi = lpTimeZoneInformation;
      TIME_ZONE_INFORMATION local_tzi;
      if (!tzi) {
        if (GetTimeZoneInformation(&local_tzi) == TIME_ZONE_ID_INVALID)
          return FALSE;
        tzi = &local_tzi;
      }

      long long fileTimeNs;
      if (!SystemTimeToFileTime(lpLocalTime, (FILETIME *)&fileTimeNs))
        return FALSE;
      fileTimeNs += tzi->Bias * 60LL * 10000000LL;
      return FileTimeToSystemTime((FILETIME *)&fileTimeNs, lpUniversalTime);
    }
  } // namespace f
} // namespace mingw_thunk
