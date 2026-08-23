#include <thunk/_common.h>
#include <thunk/stdlib.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace g
  {
    static STARTUPINFOW startup_info;
    static char *startup_info_desktop;
    static char *startup_info_title;
    static bool startup_info_initialized;
    static bool startup_info_lock;
  } // namespace g

  __DEFINE_THUNK(kernel32,
                 4,
                 VOID,
                 WINAPI,
                 GetStartupInfoA,
                 _Out_ LPSTARTUPINFOA lpStartupInfo)
  {
    while (__atomic_test_and_set(&g::startup_info_lock, __ATOMIC_ACQUIRE))
      Sleep(0);

    if (!g::startup_info_initialized) {
      STARTUPINFOW siw = {};
      siw.cb = sizeof(STARTUPINFOW);
      GetStartupInfoW(&siw);

      d::u_str u_desktop;
      if (siw.lpDesktop && u_desktop.from_w(siw.lpDesktop)) {
        g::startup_info_desktop = (char *)c::malloc(u_desktop.size() + 1);
        if (g::startup_info_desktop) {
          c::memcpy(
              g::startup_info_desktop, u_desktop.c_str(), u_desktop.size());
          g::startup_info_desktop[u_desktop.size()] = 0;
        }
      }

      d::u_str u_title;
      if (siw.lpTitle && u_title.from_w(siw.lpTitle)) {
        g::startup_info_title = (char *)c::malloc(u_title.size() + 1);
        if (g::startup_info_title) {
          c::memcpy(g::startup_info_title, u_title.c_str(), u_title.size());
          g::startup_info_title[u_title.size()] = 0;
        }
      }

      g::startup_info = siw;
      g::startup_info_initialized = true;
    }

    __atomic_clear(&g::startup_info_lock, __ATOMIC_RELEASE);

    *lpStartupInfo = STARTUPINFOA{
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
} // namespace mingw_thunk
