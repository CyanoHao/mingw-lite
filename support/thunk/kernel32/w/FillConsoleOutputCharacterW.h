#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL __stdcall
    win9x_FillConsoleOutputCharacterW(_In_ HANDLE hConsoleOutput,
                                      _In_ WCHAR cCharacter,
                                      _In_ DWORD nLength,
                                      _In_ COORD dwWriteCoord,
                                      _Out_ LPDWORD lpNumberOfCharsWritten);
  } // namespace f
} // namespace mingw_thunk
