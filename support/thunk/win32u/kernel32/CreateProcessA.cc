#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 40,
                 BOOL,
                 WINAPI,
                 CreateProcessA,
                 _In_opt_ LPCSTR lpApplicationName,
                 _Inout_opt_ LPSTR lpCommandLine,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                 _In_ BOOL bInheritHandles,
                 _In_ DWORD dwCreationFlags,
                 _In_opt_ LPVOID lpEnvironment,
                 _In_opt_ LPCSTR lpCurrentDirectory,
                 _In_ LPSTARTUPINFOA lpStartupInfo,
                 _Out_ LPPROCESS_INFORMATION lpProcessInformation)
  {
    stl::wstring w_app_name;
    if (lpApplicationName)
      w_app_name = internal::u2w(lpApplicationName);
    stl::wstring w_cmd_line;
    if (lpCommandLine)
      w_cmd_line = internal::u2w(lpCommandLine);
    stl::wstring w_current_dir;
    if (lpCurrentDirectory)
      w_current_dir = internal::u2w(lpCurrentDirectory);

    stl::wstring w_desktop;
    if (lpStartupInfo->lpDesktop)
      w_desktop = internal::u2w(lpStartupInfo->lpDesktop);
    stl::wstring w_title;
    if (lpStartupInfo->lpTitle)
      w_title = internal::u2w(lpStartupInfo->lpTitle);
    STARTUPINFOW w_startup_info = {
        sizeof(STARTUPINFOW),
        nullptr,
        lpStartupInfo->lpDesktop ? w_desktop.data() : nullptr,
        lpStartupInfo->lpTitle ? w_title.data() : nullptr,
        lpStartupInfo->dwX,
        lpStartupInfo->dwY,
        lpStartupInfo->dwXSize,
        lpStartupInfo->dwYSize,
        lpStartupInfo->dwXCountChars,
        lpStartupInfo->dwYCountChars,
        lpStartupInfo->dwFillAttribute,
        lpStartupInfo->dwFlags,
        lpStartupInfo->wShowWindow,
        0,
        nullptr,
        lpStartupInfo->hStdInput,
        lpStartupInfo->hStdOutput,
        lpStartupInfo->hStdError,
    };

    return CreateProcessW(lpApplicationName ? w_app_name.c_str() : nullptr,
                          lpCommandLine ? w_cmd_line.data() : nullptr,
                          lpProcessAttributes,
                          lpThreadAttributes,
                          bInheritHandles,
                          dwCreationFlags,
                          lpEnvironment,
                          lpCurrentDirectory ? w_current_dir.c_str() : nullptr,
                          &w_startup_info,
                          lpProcessInformation);
  }
} // namespace mingw_thunk
