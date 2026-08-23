#pragma once

// Direct console channel for the thirteen conio faces (plan §M12).
//
// Channel shape: the native faces write through WriteConsoleW /
// ReadConsoleW on the process's own STD_OUTPUT / STD_INPUT handles --
// the same handles ucrtbase uses -- so a redirected stdout fails with
// ERROR_INVALID_HANDLE exactly as the native faces do (wine anchor:
// _cputs / _putch / _cprintf all return -1 under a redirected headless
// run, and so does CONOUT$/CONIN$ being unopenable).  No fd is consumed
// and no engine channel slot is taken, which leaves the ordering of a
// direct console write against the buffered stdout channel as a known
// limit (plan D6).
//
// Text shape: our narrow strings are UTF-8, so both directions go
// through the same transcode pair the M3 wide printf/scanf shells use --
// narrow faces render UTF-8 with the musl engine and widen on the way
// out, wide faces pre-translate the format and read a console line as
// UTF-16 then narrow it for the engine.  A code point that comes back
// from the console as a lone surrogate becomes U+FFFD (M3.6-7).
//
// Internal header; only the conio shell .cc files and their tests
// include it.

#include <thunk/_no_thunk.h>
#include <thunk/unicode.h>
#include <thunk/u8crt/musl.h>

#include "../stdio/printf_shell.h"
#include "../stdio/wfmt_translate.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace i
  {
    namespace conio
    {
      /* The engine shell the printf/scanf helpers below borrow: the
       * same one the M3/M4 stdio faces use.  Referenced through its
       * qualified name because a `shell` alias here would shadow the
       * enclosing namespace name (gcc rejects the self-reference). */

      /* ------------------------- device handles ------------------------- */

      inline HANDLE out_handle() noexcept
      {
        return GetStdHandle(STD_OUTPUT_HANDLE);
      }

      inline HANDLE in_handle() noexcept
      {
        return GetStdHandle(STD_INPUT_HANDLE);
      }

      /* --------------------------- text output -------------------------- */

      /* Writes n UTF-16 units to the console, in as many WriteConsoleW
       * calls as the console asks for.  False on any failure, which is
       * the caller's whole error contract: the native faces return -1 /
       * EOF there and leave errno alone (wine anchor). */
      inline bool write_wide(const wchar_t *s, size_t n) noexcept
      {
        HANDLE h = out_handle();
        while (n > 0) {
          DWORD wrote = 0;
          if (!WriteConsoleW(h, s, (DWORD)n, &wrote, nullptr) || wrote == 0)
            return false;
          s += wrote;
          n -= wrote;
        }
        return true;
      }

      inline bool write_wide(const wchar_t *s) noexcept
      {
        return write_wide(s, wcslen(s));
      }

      /* One code point out.  Returns false when the console refuses it,
       * which _putch reports as EOF. */
      inline bool write_cp(uint32_t cp) noexcept
      {
        wchar_t one[2];
        if (cp < 0x10000) {
          one[0] = (wchar_t)cp;
          return write_wide(one, 1);
        }
        cp -= 0x10000;
        one[0] = (wchar_t)(0xD800 | (cp >> 10));
        one[1] = (wchar_t)(0xDC00 | (cp & 0x3FF));
        return write_wide(one, 2);
      }

      /* ------------------------- _putch state --------------------------- */

      /* UTF-8 analogue of the DBCS lead-byte park ucrtbase keeps in the
       * per-thread data: a multi-byte sequence cannot be written one
       * byte at a time, so the leading bytes wait here until the
       * sequence completes and then go out as one code point.  RFC 3629
       * rules: overlong starts, surrogates and out-of-range code points
       * each collapse to a single U+FFFD, and a byte that cannot
       * continue the park flushes the park and is reprocessed. */
      struct putch_state
      {
        unsigned char buf[4];
        unsigned used;
        unsigned need;
      };

      inline putch_state &putch_park() noexcept
      {
        static __thread putch_state park = {};
        return park;
      }

      /* Sequence length a first byte announces, or 0 when the byte cannot
       * start one. */
      inline unsigned u8_seq_need(unsigned char b) noexcept
      {
        if (b < 0x80)
          return 1;
        if (b >= 0xC2 && b <= 0xDF)
          return 2;
        if (b >= 0xE0 && b <= 0xEF)
          return 3;
        if (b >= 0xF0 && b <= 0xF4)
          return 4;
        return 0;
      }

      inline bool u8_is_trail(unsigned char b) noexcept
      {
        return (b & 0xC0) == 0x80;
      }

      /* Decodes a complete park, rejecting the encodings RFC 3629
       * excludes (overlongs, UTF-16 surrogates, > U+10FFFF). */
      inline uint32_t putch_decode(const unsigned char *b, unsigned n) noexcept
      {
        uint32_t cp;
        switch (n) {
        case 1:
          return b[0];
        case 2:
          cp = (uint32_t)(b[0] & 0x1F);
          break;
        case 3:
          cp = (uint32_t)(b[0] & 0x0F);
          break;
        default:
          cp = (uint32_t)(b[0] & 0x07);
          break;
        }

        for (unsigned i = 1; i < n; ++i)
          cp = (cp << 6) | (uint32_t)(b[i] & 0x3F);

        /* the overlong two-byte forms of the ASCII range.  0xC0/0xC1
         * never reach here (u8_seq_need refuses them as lead bytes), but
         * the decoder is total on its own so the guard is kept */
        if (n == 2 && cp < 0x80)
          return g::u16_rep;

        /* the four second-byte ranges that carry the exclusions */
        if (n == 3 && b[0] == 0xE0 && b[1] < 0xA0)
          return g::u16_rep;
        if (n == 3 && b[0] == 0xED && b[1] > 0x9F)
          return g::u16_rep;
        if (n == 4 && b[0] == 0xF0 && b[1] < 0x90)
          return g::u16_rep;
        if (n == 4 && b[0] == 0xF4 && b[1] > 0x8F)
          return g::u16_rep;

        return cp;
      }

      /* _putch / _putch_nolock body: returns the character on success
       * (including a parked lead byte, exactly as the reference returns
       * it without touching the device) and EOF when the code point
       * could not be written. */
      inline int putch(int ch) noexcept
      {
        unsigned char c = (unsigned char)ch;
        putch_state &park = putch_park();

        if (park.used == 0) {
          unsigned need = u8_seq_need(c);
          if (need == 0) {
            /* not a sequence start (a stray trail byte, 0xC0/0xC1, or
             * 0xF5..0xFF): one replacement byte stands for it */
            park.used = 0;
            return write_cp(g::u16_rep) ? ch : EOF;
          }
          if (need == 1)
            return write_cp(c) ? ch : EOF;
          park.buf[0] = c;
          park.need = need;
          park.used = 1;
          return ch;
        }

        if (!u8_is_trail(c)) {
          /* the park cannot continue: flush it and start over on c */
          park.used = 0;
          return write_cp(g::u16_rep) ? putch(ch) : EOF;
        }

        park.buf[park.used++] = c;
        if (park.used < park.need)
          return ch;

        uint32_t cp = putch_decode(park.buf, park.need);
        park.used = 0;
        return write_cp(cp) ? ch : EOF;
      }

      /* --------------------------- text input --------------------------- */

      /* Line-granular console read.  ENABLE_LINE_INPUT is what the
       * reference asks for; unlike the native per-character adapter
       * this cannot straddle a newline inside one _cgets call, which is
       * the documented 行粒度 limit of the conio scanf family.  The
       * trailing CRLF / LF / CR is stripped the way _cgetws_s strips
       * it, so `units` counts payload only. */
      inline bool read_cooked_line(wchar_t *buf, size_t cap, size_t *units) noexcept
      {
        if (cap < 2)
          return false;

        HANDLE h = in_handle();

        DWORD mode = 0;
        if (!GetConsoleMode(h, &mode))
          return false;

        DWORD cooked = (mode | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
                        ENABLE_ECHO_INPUT) &
                       ~ENABLE_VIRTUAL_TERMINAL_INPUT;
        bool retuned = SetConsoleMode(h, cooked) != 0;

        DWORD read = 0;
        BOOL ok = ReadConsoleW(h, buf, (DWORD)(cap - 1), &read, nullptr);

        if (retuned)
          SetConsoleMode(h, mode);

        if (!ok)
          return false;

        size_t n = read;
        if (n > 0 && buf[n - 1] == L'\n')
          --n;
        if (n > 0 && buf[n - 1] == L'\r')
          --n;

        *units = n;
        return true;
      }

      /* -------------------- printf render (no device) -------------------- */

      /* Rendered console text as UTF-16 units: the engine's UTF-8 output
       * widened once, ready for WriteConsoleW.  A render failure and a
       * device failure are separate so the faces can tell them apart. */
      struct render_out
      {
        wchar_t stackbuf[256];
        wchar_t *buf;
        size_t units;
        bool heap;
        bool ok;

        render_out() noexcept : buf(stackbuf), units(0), heap(false), ok(false)
        {
        }

        ~render_out()
        {
          if (heap)
            free(buf);
        }

        render_out(const render_out &) = delete;
        render_out &operator=(const render_out &) = delete;

        /* Widens the engine's UTF-8 bytes into this holder. */
        bool carry(const char *u8, size_t bytes) noexcept
        {
          size_t need = i::shell::u8_to_u16(u8, bytes, nullptr, 0, false);
          if (need + 1 > sizeof stackbuf) {
            buf = (wchar_t *)malloc((need + 1) * sizeof(wchar_t));
            if (buf == nullptr)
              return false;
            heap = true;
          }
          i::shell::u8_to_u16(u8, bytes, buf, need + 1, true);
          units = need;
          ok = true;
          return true;
        }
      };

      /* Narrow printf render: the musl engine with the caller's UTF-8
       * format, widened for the console.  Mirrors the gates of
       * __stdio_common_vfprintf -- null args and the 0x10
       * three-digit-exponent guard (%n rides the engine's musl
       * semantics: always enabled). */
      inline bool render_narrow(uint64_t options,
                                const char *format,
                                va_list ap,
                                render_out &out,
                                int *err) noexcept
      {
        if (format == nullptr) {
          *err = EINVAL;
          return false;
        }

        i::shell::exp_guard guard(options);
        i::shell::wide_render r(format, ap);
        if (!r.ok) {
          *err = ENOMEM;
          return false;
        }
        return out.carry(r.buf, r.bytes);
      }

      /* Wide printf render: the M3 pre-translator first (0x4 selects the
       * legacy wide conventions), then the same engine render. */
      inline bool render_wide(uint64_t options,
                              const wchar_t *format,
                              va_list ap,
                              render_out &out,
                              int *err) noexcept
      {
        if (format == nullptr) {
          *err = EINVAL;
          return false;
        }

        i::shell::wfmt_holder nfmt(format, false, (options & 0x4) != 0);
        if (!nfmt.ok) {
          *err = ENOMEM;
          return false;
        }

        i::shell::exp_guard guard(options);
        i::shell::wide_render r(nfmt.nfmt, ap);
        if (!r.ok) {
          *err = ENOMEM;
          return false;
        }
        return out.carry(r.buf, r.bytes);
      }

      /* One console write for a rendered face: the reference's return
       * value is the number of characters handed to the console, which
       * for both the narrow and the wide family is the UTF-16 unit
       * count.  A device failure is -1 with errno untouched. */
      inline int emit(render_out &out) noexcept
      {
        if (!out.ok)
          return -1;
        if (!write_wide(out.buf, out.units))
          return -1;
        return (int)out.units;
      }

      /* ------------------------- scanf render ---------------------------- */

      /* Line-granular scanf.  The console line arrives as UTF-16, is
       * narrowed for the engine, and the engine writes the caller's
       * (wide, for the vcw* half) destinations.  `nfmt` is the
       * pre-translated narrow format for the wide family and the caller's
       * UTF-8 format for the narrow one. */
      inline int scan_line_narrow(const char *nfmt, va_list ap) noexcept
      {
        wchar_t line[128];
        size_t units = 0;
        if (!read_cooked_line(line, 128, &units))
          return EOF;

        char narrow[3 * 128 + 1];
        size_t bytes = i::shell::u16_to_u8(line, units, narrow);
        return musl::vsscanf(narrow, nfmt, ap);
      }

      inline int scan_line_wide(const wchar_t *format, va_list ap) noexcept
      {
        i::shell::wfmt_holder nfmt(format, true, false);
        if (!nfmt.ok) {
          errno = ENOMEM;
          return EOF;
        }
        return scan_line_narrow(nfmt.nfmt, ap);
      }

      /* ------------------------- _cgets protocol ------------------------- */

      /* Unconsumed console input.  The reference keeps one character in
       * __console_wchar_buffer so a code point that did not fit the
       * caller's buffer comes back on the next call; ours holds the rest
       * of the line, in UTF-16 units, for the same reason. */
      struct line_stash
      {
        wchar_t buf[128];
        size_t n;
        size_t pos;
      };

      inline line_stash &cgets_stash() noexcept
      {
        static __thread line_stash stash = {};
        return stash;
      }

      /* Hands the next up-to-cap payload units to the caller, from the
       * stash when anything is left in it and from a fresh console line
       * otherwise. */
      inline bool take_line(line_stash &stash,
                            wchar_t *buf,
                            size_t cap,
                            size_t *units) noexcept
      {
        if (stash.pos >= stash.n) {
          stash.pos = stash.n = 0;
          if (!read_cooked_line(stash.buf, 128, &stash.n))
            return false;
        }

        size_t take = stash.n - stash.pos;
        if (take > cap)
          take = cap;
        memcpy(buf, stash.buf + stash.pos, take * sizeof(wchar_t));
        stash.pos += take;
        *units = take;
        return true;
      }

      /* Puts units back at the head of the stash.  take_line always
       * drains the stash in one go (a console line is at most 127 units
       * and the cap is 128), so there is never anything left behind and
       * the pushed-back units simply become the whole stash.  That is
       * the reference's pushback, widened from one character to the
       * rest of the line. */
      inline void unget_line(line_stash &stash,
                             const wchar_t *buf,
                             size_t from,
                             size_t n) noexcept
      {
        if (n == 0)
          return;

        memcpy(stash.buf, buf + from, n * sizeof(wchar_t));
        stash.n = n;
        stash.pos = 0;
      }

      inline size_t cp_u8_len(uint32_t cp) noexcept
      {
        if (cp < 0x80)
          return 1;
        if (cp < 0x800)
          return 2;
        if (cp < 0x10000)
          return 3;
        return 4;
      }

      /* A console read that did not happen is reported the way the
       * reference reports it: the Win32 code mapped into an errno.  A
       * redirected or absent console fails with ERROR_INVALID_HANDLE,
       * which is EBADF. */
      inline int read_errno() noexcept
      {
        return GetLastError() == ERROR_INVALID_HANDLE ? EBADF : EIO;
      }

      /* The shared body of the two _cgets faces (conio/cgets.cpp, minus
       * the DBCS code-page round trip: the console line is already
       * UTF-16, so the only conversion left is the narrow one the
       * reference does per character).
       *
       * The protocol is kept as-is -- the buffer is cleared first, the
       * payload is NUL-terminated inside `count` bytes, a code point
       * that would not fit is left out entirely rather than truncated,
       * and whatever did not fit goes back into the stash so the next
       * call sees it.  Returns 0 and leaves errno alone on success; on
       * failure returns the errno_t it also stored in errno.
       *
       * wine exports _cgets_s but has no implementation of it, so the
       * readable contract is the reference; the headless wine anchor for
       * the family is _cgets, which returns null and leaves the string
       * empty. */
      inline errno_t cgets_s_body(char *buffer,
                                  size_t count,
                                  size_t *size_read) noexcept
      {
        if (buffer == nullptr || count == 0 || size_read == nullptr) {
          errno = EINVAL;
          return EINVAL;
        }

        *size_read = 0;
        memset(buffer, 0, count); // _RESET_STRING

        line_stash &stash = cgets_stash();

        wchar_t line[128];
        size_t units = 0;
        if (!take_line(stash, line, 128, &units)) {
          int err = read_errno();
          errno = err;
          return err;
        }

        size_t avail = count - 1;
        size_t got = 0;
        size_t i = 0;
        size_t at = 0;
        while (i < units) {
          // `at` is where the code point under consideration starts, so
          // the pushback below can hand back a code point the fit check
          // refused rather than skipping it
          at = i;
          const wchar_t *p = line + i;
          uint32_t cp = i::shell::u16_next_cp(p, line + units);
          i = size_t(p - line);

          if (got + cp_u8_len(cp) > avail)
            break;

          i::shell::utf8_emit([&got, out = buffer](char c) { out[got++] = c; },
                               cp);
        }

        unget_line(stash, line, at, units - at);

        buffer[got] = '\0';
        *size_read = got;
        return 0;
      }
    } // namespace conio
  } // namespace i
} // namespace mingw_thunk
