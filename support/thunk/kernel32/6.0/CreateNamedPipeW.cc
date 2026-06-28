#include "CreateNamedPipeW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>

#include <namedpipeapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 32,
                 HANDLE,
                 WINAPI,
                 CreateNamedPipeW,
                 _In_ LPCWSTR lpName,
                 _In_ DWORD dwOpenMode,
                 _In_ DWORD dwPipeMode,
                 _In_ DWORD nMaxInstances,
                 _In_ DWORD nOutBufferSize,
                 _In_ DWORD nInBufferSize,
                 _In_ DWORD nDefaultTimeOut,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes)
  {
    __DISPATCH_THUNK_2(CreateNamedPipeW,
                       i::os_version() >= g::win32_vista,
                       __ms_CreateNamedPipeW,
                       &f::fallback_CreateNamedPipeW);

    return dllimport_CreateNamedPipeW(lpName,
                                      dwOpenMode,
                                      dwPipeMode,
                                      nMaxInstances,
                                      nOutBufferSize,
                                      nInBufferSize,
                                      nDefaultTimeOut,
                                      lpSecurityAttributes);
  }

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
                                                lpSecurityAttributes)
    {
      dwPipeMode &= ~PIPE_REJECT_REMOTE_CLIENTS;
      return __ms_CreateNamedPipeW(lpName,
                                   dwOpenMode,
                                   dwPipeMode,
                                   nMaxInstances,
                                   nOutBufferSize,
                                   nInBufferSize,
                                   nDefaultTimeOut,
                                   lpSecurityAttributes);
    }
  } // namespace f
} // namespace mingw_thunk
