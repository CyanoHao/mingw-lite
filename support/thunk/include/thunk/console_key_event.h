#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace d
  {
    struct console_key_event_pending_bytes
    {
      KEY_EVENT_RECORD event;
      char bytes[3];
      int count;

      DWORD flush(PINPUT_RECORD buf, DWORD room) noexcept
      {
        DWORD written = 0;
        while (count > 0 && written < room) {
          KEY_EVENT_RECORD e = event;
          e.uChar.AsciiChar = bytes[0];
          buf->EventType = KEY_EVENT;
          buf->Event.KeyEvent = e;
          ++buf;
          ++written;
          --count;
          for (int j = 0; j < count; ++j)
            bytes[j] = bytes[j + 1];
        }
        return written;
      }

      DWORD peek(PINPUT_RECORD buf, DWORD room) const noexcept
      {
        DWORD written = 0;
        for (int j = 0; j < count && written < room; ++j) {
          KEY_EVENT_RECORD e = event;
          e.uChar.AsciiChar = bytes[j];
          buf->EventType = KEY_EVENT;
          buf->Event.KeyEvent = e;
          ++buf;
          ++written;
        }
        return written;
      }

      void store(const KEY_EVENT_RECORD &rec, const char *rest, int n) noexcept
      {
        KEY_EVENT_RECORD tmpl = rec;
        tmpl.wVirtualKeyCode = 0;
        tmpl.wVirtualScanCode = 0;
        event = tmpl;
        for (int j = 0; j < n; ++j)
          bytes[j] = rest[j];
        count = n;
      }
    };
  } // namespace d

  namespace g
  {
    inline d::console_key_event_pending_bytes console_key_event_pending_bytes;
  }
} // namespace mingw_thunk
