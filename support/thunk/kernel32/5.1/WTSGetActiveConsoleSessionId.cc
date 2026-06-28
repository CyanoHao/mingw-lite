#include "WTSGetActiveConsoleSessionId.h"

#include <thunk/_common.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32, 0, DWORD, WINAPI, WTSGetActiveConsoleSessionId)
  {
    __DISPATCH_THUNK_2(WTSGetActiveConsoleSessionId,
                       const auto pfn = try_get_WTSGetActiveConsoleSessionId(),
                       pfn,
                       &f::fallback_WTSGetActiveConsoleSessionId);

    return dllimport_WTSGetActiveConsoleSessionId();
  }

  namespace f
  {
    DWORD WINAPI fallback_WTSGetActiveConsoleSessionId()
    {
      return 0xFFFFFFFF;
    }
  } // namespace f
} // namespace mingw_thunk
