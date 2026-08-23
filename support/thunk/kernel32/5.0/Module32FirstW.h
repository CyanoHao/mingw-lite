#pragma once

#include <windows.h>

// after <windows.h>
#include <tlhelp32.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI winme_Module32FirstW(_In_ HANDLE hSnapshot,
                                     _Inout_ LPMODULEENTRY32W lpme);
    BOOL WINAPI noop_Module32FirstW(_In_ HANDLE hSnapshot,
                                    _Inout_ LPMODULEENTRY32W lpme);
  } // namespace f
} // namespace mingw_thunk
