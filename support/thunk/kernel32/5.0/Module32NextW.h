#pragma once

#include <windows.h>

// after <windows.h>
#include <tlhelp32.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI winme_Module32NextW(_In_ HANDLE hSnapshot,
                                    _Out_ LPMODULEENTRY32W lpme);
    BOOL WINAPI noop_Module32NextW(_In_ HANDLE hSnapshot,
                                   _Out_ LPMODULEENTRY32W lpme);
  } // namespace f
} // namespace mingw_thunk
