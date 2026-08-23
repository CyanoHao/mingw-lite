#include "PeekConsoleInputW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 16,
                 BOOL,
                 WINAPI,
                 PeekConsoleInputW,
                 _In_ HANDLE hConsoleInput,
                 _Out_ PINPUT_RECORD lpBuffer,
                 _In_ DWORD nLength,
                 _Out_ LPDWORD lpNumberOfEventsRead)
  {
    __DISPATCH_THUNK_2(PeekConsoleInputW,
                       i::is_nt(),
                       &__ms_PeekConsoleInputW,
                       &f::win9x_PeekConsoleInputW);

    return dllimport_PeekConsoleInputW(
        hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
  }

  namespace f
  {
    // Windows 9x: no DBCS support.
    // CJK input can be enabled by the command `pdos95`.
    // Take pinyin for example: 中文 (z-h-o-n-g-w-e-n-sp),
    // the program reads actually the key sequence, and nothing on committing.
    BOOL __stdcall win9x_PeekConsoleInputW(_In_ HANDLE hConsoleInput,
                                           _Out_ PINPUT_RECORD lpBuffer,
                                           _In_ DWORD nLength,
                                           _Out_ LPDWORD lpNumberOfEventsRead)
    {
      DWORD read = 0;
      if (!__ms_PeekConsoleInputA(hConsoleInput, lpBuffer, nLength, &read))
        return FALSE;

      for (DWORD i = 0; i < read; ++i) {
        if (lpBuffer[i].EventType != KEY_EVENT)
          continue;
        WCHAR ch = L'?';
        __ms_MultiByteToWideChar(CP_OEMCP,
                                 0,
                                 &lpBuffer[i].Event.KeyEvent.uChar.AsciiChar,
                                 1,
                                 &ch,
                                 1);
        lpBuffer[i].Event.KeyEvent.uChar.UnicodeChar = ch;
      }

      if (lpNumberOfEventsRead)
        *lpNumberOfEventsRead = read;
      return TRUE;
    }
  } // namespace f
} // namespace mingw_thunk
