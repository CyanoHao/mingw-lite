#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 20,
                 BOOL,
                 WINAPI,
                 FillConsoleOutputCharacterA,
                 _In_ HANDLE hConsoleOutput,
                 _In_ CHAR cCharacter,
                 _In_ DWORD nLength,
                 _In_ COORD dwWriteCoord,
                 _Out_ LPDWORD lpNumberOfCharsWritten)
  {
    WCHAR wch = L'?';
    if ((BYTE)cCharacter < 0x80)
      wch = (WCHAR)(BYTE)cCharacter;

    return FillConsoleOutputCharacterW(
        hConsoleOutput, wch, nLength, dwWriteCoord, lpNumberOfCharsWritten);
  }
} // namespace mingw_thunk
