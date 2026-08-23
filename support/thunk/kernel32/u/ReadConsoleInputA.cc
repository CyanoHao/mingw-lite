#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/console_key_event.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace
  {
    int encode_utf8(WCHAR ch, char out[4]) noexcept
    {
      return WideCharToMultiByte(CP_UTF8, 0, &ch, 1, out, 4, NULL, NULL);
    }
  } // namespace

  __DEFINE_THUNK(kernel32,
                 16,
                 BOOL,
                 WINAPI,
                 ReadConsoleInputA,
                 _In_ HANDLE hConsoleInput,
                 _Out_ PINPUT_RECORD lpBuffer,
                 _In_ DWORD nLength,
                 _Out_ LPDWORD lpNumberOfEventsRead)
  {
    if (!lpBuffer || !lpNumberOfEventsRead || nLength == 0) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    DWORD filled = 0;

    filled += g::console_key_event_pending_bytes.flush(lpBuffer, nLength);

    for (; filled < nLength;) {
      INPUT_RECORD rec;
      DWORD read = 0;
      if (!ReadConsoleInputW(hConsoleInput, &rec, 1, &read) || read == 0)
        break;

      if (rec.EventType != KEY_EVENT) {
        lpBuffer[filled++] = rec;
        continue;
      }

      char bytes[4];
      int cnt = encode_utf8(rec.Event.KeyEvent.uChar.UnicodeChar, bytes);
      if (cnt <= 0) {
        bytes[0] = '?';
        cnt = 1;
      }

      KEY_EVENT_RECORD first = rec.Event.KeyEvent;
      first.uChar.AsciiChar = bytes[0];
      lpBuffer[filled].EventType = KEY_EVENT;
      lpBuffer[filled].Event.KeyEvent = first;
      ++filled;

      for (int i = 1; i < cnt; ++i) {
        if (filled < nLength) {
          KEY_EVENT_RECORD cont = rec.Event.KeyEvent;
          cont.uChar.AsciiChar = bytes[i];
          cont.wVirtualKeyCode = 0;
          cont.wVirtualScanCode = 0;
          lpBuffer[filled].EventType = KEY_EVENT;
          lpBuffer[filled].Event.KeyEvent = cont;
          ++filled;
        } else {
          g::console_key_event_pending_bytes.store(
              rec.Event.KeyEvent, bytes + i, cnt - i);
          break;
        }
      }
    }

    if (filled == 0)
      return FALSE;

    *lpNumberOfEventsRead = filled;
    return TRUE;
  }
} // namespace mingw_thunk
