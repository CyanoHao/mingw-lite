#pragma once

#include <windows.h>

// after <windows.h>
#include <namedpipeapi.h>

namespace mingw_thunk
{
  namespace f
  {
    HANDLE WINAPI fallback_CreateNamedPipeW(_In_ LPCWSTR lpName,
                                            _In_ DWORD dwOpenMode,
                                            _In_ DWORD dwPipeMode,
                                            _In_ DWORD nMaxInstances,
                                            _In_ DWORD nOutBufferSize,
                                            _In_ DWORD nInBufferSize,
                                            _In_ DWORD nDefaultTimeOut,
                                            _In_opt_ LPSECURITY_ATTRIBUTES
                                                lpSecurityAttributes);
  }
} // namespace mingw_thunk
