#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL __stdcall win9x_PeekConsoleInputW(_In_ HANDLE hConsoleInput,
                                           _Out_ PINPUT_RECORD lpBuffer,
                                           _In_ DWORD nLength,
                                           _Out_ LPDWORD lpNumberOfEventsRead);
  } // namespace f
} // namespace mingw_thunk
