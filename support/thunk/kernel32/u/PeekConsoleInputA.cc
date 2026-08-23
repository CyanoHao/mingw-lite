#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/console_key_event.h>
#include <thunk/stdlib.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace
  {
    int encode_utf8(WCHAR ch, char out[4]) noexcept
    {
      return WideCharToMultiByte(CP_UTF8, 0, &ch, 1, out, 4, NULL, NULL);
    }

    struct mem_guard
    {
      void *ptr;
      explicit mem_guard(void *p) noexcept : ptr(p)
      {
      }
      ~mem_guard()
      {
        c::free(ptr);
      }
      mem_guard(const mem_guard &) = delete;
      mem_guard &operator=(const mem_guard &) = delete;
    };
  } // namespace

  __DEFINE_THUNK(kernel32,
                 16,
                 BOOL,
                 WINAPI,
                 PeekConsoleInputA,
                 _In_ HANDLE hConsoleInput,
                 _Out_ PINPUT_RECORD lpBuffer,
                 _In_ DWORD nLength,
                 _Out_ LPDWORD lpNumberOfEventsRead)
  {
    if (!lpBuffer || !lpNumberOfEventsRead || nLength == 0) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    DWORD filled = g::console_key_event_pending_bytes.peek(lpBuffer, nLength);

    DWORD room = nLength - filled;
    if (room == 0) {
      *lpNumberOfEventsRead = filled;
      return TRUE;
    }

    if (room > SIZE_MAX / sizeof(INPUT_RECORD)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    INPUT_RECORD stack_rec;
    void *mem = NULL;
    if (room > 1) {
      mem = c::malloc(room * sizeof(INPUT_RECORD));
      if (!mem) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }
    }
    mem_guard guard(mem);

    INPUT_RECORD *temp = mem ? static_cast<INPUT_RECORD *>(mem) : &stack_rec;

    DWORD read = 0;
    if (!PeekConsoleInputW(hConsoleInput, temp, room, &read))
      return FALSE;

    for (DWORD i = 0; i < read && filled < nLength; ++i) {
      if (temp[i].EventType != KEY_EVENT) {
        lpBuffer[filled++] = temp[i];
        continue;
      }

      char bytes[4];
      int cnt = encode_utf8(temp[i].Event.KeyEvent.uChar.UnicodeChar, bytes);
      if (cnt <= 0) {
        bytes[0] = '?';
        cnt = 1;
      }

      for (int j = 0; j < cnt && filled < nLength; ++j) {
        KEY_EVENT_RECORD e = temp[i].Event.KeyEvent;
        e.uChar.AsciiChar = bytes[j];
        if (j > 0) {
          e.wVirtualKeyCode = 0;
          e.wVirtualScanCode = 0;
        }
        lpBuffer[filled].EventType = KEY_EVENT;
        lpBuffer[filled].Event.KeyEvent = e;
        ++filled;
      }
    }

    *lpNumberOfEventsRead = filled;
    return TRUE;
  }
} // namespace mingw_thunk
