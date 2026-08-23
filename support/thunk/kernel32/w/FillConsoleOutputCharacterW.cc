#include "FillConsoleOutputCharacterW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 20,
                 BOOL,
                 WINAPI,
                 FillConsoleOutputCharacterW,
                 _In_ HANDLE hConsoleOutput,
                 _In_ WCHAR cCharacter,
                 _In_ DWORD nLength,
                 _In_ COORD dwWriteCoord,
                 _Out_ LPDWORD lpNumberOfCharsWritten)
  {
    __DISPATCH_THUNK_2(FillConsoleOutputCharacterW,
                       i::is_nt(),
                       &__ms_FillConsoleOutputCharacterW,
                       &f::win9x_FillConsoleOutputCharacterW);

    return dllimport_FillConsoleOutputCharacterW(hConsoleOutput,
                                                 cCharacter,
                                                 nLength,
                                                 dwWriteCoord,
                                                 lpNumberOfCharsWritten);
  }

  namespace f
  {
    BOOL __stdcall
    win9x_FillConsoleOutputCharacterW(_In_ HANDLE hConsoleOutput,
                                      _In_ WCHAR cCharacter,
                                      _In_ DWORD nLength,
                                      _In_ COORD dwWriteCoord,
                                      _Out_ LPDWORD lpNumberOfCharsWritten)
    {
      CHAR ch = '?';
      __ms_WideCharToMultiByte(CP_OEMCP, 0, &cCharacter, 1, &ch, 1, NULL, NULL);
      return __ms_FillConsoleOutputCharacterA(
          hConsoleOutput, ch, nLength, dwWriteCoord, lpNumberOfCharsWritten);
    }
  } // namespace f
} // namespace mingw_thunk
