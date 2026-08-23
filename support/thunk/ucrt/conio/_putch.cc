#include <thunk/_common.h>

#include "conio_channel.h"

namespace mingw_thunk
{
  // conio/putch.cpp.  The reference delegates to _putch_nolock, so the
  // two faces share one body and differ only in the lock the native
  // takes; there is no lock to take here because the write is a single
  // WriteConsoleW of at most two units, and two _putch calls in a row
  // are not atomic in the native either.
  //
  // UTF-8 semantics (the DBCS lead-byte park, in UTF-8 terms): a leading
  // byte of a multi-byte sequence returns the character without touching
  // the device, exactly as the reference returns a DBCS lead byte; the
  // byte that completes the sequence writes the decoded code point and
  // returns the character, or EOF when the console refuses it.  A
  // sequence that never completes is flushed as U+FFFD when the next
  // byte cannot continue it.
  //
  // wine anchor: every byte returns EOF under a redirected headless run
  // (including 0x80, 0xE4 and 0xFF), because the reference's own
  // isleadbyte() is false for those in the ANSI code page.  Under
  // C.UTF-8 the 0xC2..0xF4 leading bytes park and return the character
  // instead, which is the documented divergence.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _putch,
                 int ch)
  {
    return i::conio::putch(ch);
  }
} // namespace mingw_thunk
