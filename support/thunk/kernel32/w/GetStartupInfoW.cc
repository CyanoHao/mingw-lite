#include "GetStartupInfoW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/stdlib.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 VOID,
                 WINAPI,
                 GetStartupInfoW,
                 _Out_ LPSTARTUPINFOW lpStartupInfo)
  {
    __DISPATCH_THUNK_2(GetStartupInfoW,
                       i::is_nt(),
                       &__ms_GetStartupInfoW,
                       &f::win9x_GetStartupInfoW);

    return dllimport_GetStartupInfoW(lpStartupInfo);
  }

  namespace g
  {
    static STARTUPINFOA startup_info;
    static wchar_t *startup_info_desktop;
    static wchar_t *startup_info_title;
    static bool startup_info_initialized;
    static bool startup_info_lock;
  } // namespace g

  namespace f
  {
    VOID __stdcall win9x_GetStartupInfoW(_Out_ LPSTARTUPINFOW lpStartupInfo)
    {
      while (__atomic_test_and_set(&g::startup_info_lock, __ATOMIC_ACQUIRE))
        ;

      if (!g::startup_info_initialized) {
        STARTUPINFOA sia = {};
        sia.cb = sizeof(STARTUPINFOA);
        __ms_GetStartupInfoA(&sia);

        d::w_str w_desktop;
        if (sia.lpDesktop && w_desktop.from_a(sia.lpDesktop)) {
          g::startup_info_desktop = (wchar_t *)c::malloc(
              (c::wcslen(w_desktop.c_str()) + 1) * sizeof(wchar_t));
          if (g::startup_info_desktop)
            c::wcscpy(g::startup_info_desktop, w_desktop.c_str());
        }

        d::w_str w_title;
        if (sia.lpTitle && w_title.from_a(sia.lpTitle)) {
          g::startup_info_title = (wchar_t *)c::malloc(
              (c::wcslen(w_title.c_str()) + 1) * sizeof(wchar_t));
          if (g::startup_info_title)
            c::wcscpy(g::startup_info_title, w_title.c_str());
        }

        g::startup_info = sia;
        g::startup_info_initialized = true;
      }

      __atomic_clear(&g::startup_info_lock, __ATOMIC_RELEASE);

      *lpStartupInfo = STARTUPINFOW{
          lpStartupInfo->cb,
          nullptr,
          g::startup_info_desktop,
          g::startup_info_title,
          g::startup_info.dwX,
          g::startup_info.dwY,
          g::startup_info.dwXSize,
          g::startup_info.dwYSize,
          g::startup_info.dwXCountChars,
          g::startup_info.dwYCountChars,
          g::startup_info.dwFillAttribute,
          g::startup_info.dwFlags,
          g::startup_info.wShowWindow,
          0,
          nullptr,
          g::startup_info.hStdInput,
          g::startup_info.hStdOutput,
          g::startup_info.hStdError,
      };
    }
  } // namespace f
} // namespace mingw_thunk
