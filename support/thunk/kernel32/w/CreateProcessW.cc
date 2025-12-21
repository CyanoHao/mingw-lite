#include "CreateProcessW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 40,
                 BOOL,
                 WINAPI,
                 CreateProcessW,
                 _In_opt_ LPCWSTR lpApplicationName,
                 _Inout_opt_ LPWSTR lpCommandLine,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                 _In_ BOOL bInheritHandles,
                 _In_ DWORD dwCreationFlags,
                 _In_opt_ LPVOID lpEnvironment,
                 _In_opt_ LPCWSTR lpCurrentDirectory,
                 _In_ LPSTARTUPINFOW lpStartupInfo,
                 _Out_ LPPROCESS_INFORMATION lpProcessInformation)
  {
    if (internal::is_nt())
      return get_CreateProcessW()(lpApplicationName,
                                  lpCommandLine,
                                  lpProcessAttributes,
                                  lpThreadAttributes,
                                  bInheritHandles,
                                  dwCreationFlags,
                                  lpEnvironment,
                                  lpCurrentDirectory,
                                  lpStartupInfo,
                                  lpProcessInformation);

    return impl::win9x_CreateProcessW(lpApplicationName,
                                      lpCommandLine,
                                      lpProcessAttributes,
                                      lpThreadAttributes,
                                      bInheritHandles,
                                      dwCreationFlags,
                                      lpEnvironment,
                                      lpCurrentDirectory,
                                      lpStartupInfo,
                                      lpProcessInformation);
  }

  namespace impl
  {
    BOOL
    win9x_CreateProcessW(_In_opt_ LPCWSTR lpApplicationName,
                         _Inout_opt_ LPWSTR lpCommandLine,
                         _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                         _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                         _In_ BOOL bInheritHandles,
                         _In_ DWORD dwCreationFlags,
                         _In_opt_ LPVOID lpEnvironment,
                         _In_opt_ LPCWSTR lpCurrentDirectory,
                         _In_ LPSTARTUPINFOW lpStartupInfo,
                         _Out_ LPPROCESS_INFORMATION lpProcessInformation)
    {
      stl::string a_app_name;
      if (lpApplicationName)
        a_app_name = internal::w2a(lpApplicationName);
      stl::string a_cmd_line;
      if (lpCommandLine)
        a_cmd_line = internal::w2a(lpCommandLine);
      stl::string a_env;
      if (lpEnvironment) {
        wchar_t *env = (wchar_t *)lpEnvironment;
        int len = 0;
        bool prev_is_nul = false;
        while (true) {
          if (env[len] == 0) {
            if (prev_is_nul)
              break;
            prev_is_nul = true;
          } else {
            prev_is_nul = false;
          }
          len++;
        }
        a_env = internal::w2a(env, len);
      }
      stl::string a_cwd;
      if (lpCurrentDirectory)
        a_cwd = internal::w2a(lpCurrentDirectory);

      stl::string a_desktop;
      if (lpStartupInfo->lpDesktop)
        a_desktop = internal::w2a(lpStartupInfo->lpDesktop);
      stl::string a_title;
      if (lpStartupInfo->lpTitle)
        a_title = internal::w2a(lpStartupInfo->lpTitle);
      STARTUPINFOA si{
          .cb = sizeof(STARTUPINFOA),
          .lpReserved = nullptr,
          .lpDesktop = lpStartupInfo->lpDesktop ? a_desktop.data() : nullptr,
          .lpTitle = lpStartupInfo->lpTitle ? a_title.data() : nullptr,
          .dwX = lpStartupInfo->dwX,
          .dwY = lpStartupInfo->dwY,
          .dwXSize = lpStartupInfo->dwXSize,
          .dwYSize = lpStartupInfo->dwYSize,
          .dwXCountChars = lpStartupInfo->dwXCountChars,
          .dwYCountChars = lpStartupInfo->dwYCountChars,
          .dwFillAttribute = lpStartupInfo->dwFillAttribute,
          .dwFlags = lpStartupInfo->dwFlags,
          .wShowWindow = lpStartupInfo->wShowWindow,
          .cbReserved2 = 0,
          .lpReserved2 = nullptr,
          .hStdInput = lpStartupInfo->hStdInput,
          .hStdOutput = lpStartupInfo->hStdOutput,
          .hStdError = lpStartupInfo->hStdError,
      };

      return __ms_CreateProcessA(lpApplicationName ? a_app_name.data()
                                                   : nullptr,
                                 lpCommandLine ? a_cmd_line.data() : nullptr,
                                 lpProcessAttributes,
                                 lpThreadAttributes,
                                 bInheritHandles,
                                 dwCreationFlags,
                                 lpEnvironment ? a_env.data() : nullptr,
                                 lpCurrentDirectory ? a_cwd.data() : nullptr,
                                 &si,
                                 lpProcessInformation);
    }
  } // namespace impl
} // namespace mingw_thunk
