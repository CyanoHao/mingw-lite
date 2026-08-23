#pragma once

// Wide-family pre-translator and UTF-8 <-> UTF-16 transcode helpers
// (plan §M3): the __stdio_common_v*w* shells translate the wide format
// string into a narrow (UTF-8) format consumed by the engine with the
// SAME va_list — the caller laid the arguments out per the wide
// conventions, the directive rewrites realign each s/c-family argument
// to what the narrow engine expects.  Text runs are transcoded
// verbatim (surrogate pairs included).
//
// Conventions (wine ucrtbase experiment, plan M3.1):
//  - printf, options bit 0x4 (LEGACY_WIDE_SPECIFIERS): bare %s/%c are
//    wide arguments -> %ls/%lc, %hs/%hc are narrow -> %s/%c, %ws ->
//    %ls, %S/%C are narrow -> %s/%c
//  - printf without 0x4 (ISO/C99 — every product wrapper): zero
//    rewrite; the engine's musl semantics already match (%s takes
//    char*, %c takes int, %S/%C stay engine-native)
//  - scanf (options are irrelevant — both conventions write wide):
//    bare %s/%c/%[ produce wchar_t -> %ls/%lc/%l[, and %hs/%hc/%h[
//    stay narrow
//
// Internal header; only the stdio shell .cc files and their tests
// include it.

#include <thunk/u8crt/musl.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace i
  {
    namespace shell
    {
      /* ---------------- UTF-8 <-> UTF-16 transcode ---------------- */

      /* Decode one code point from [p, end).  Unpaired surrogates
       * yield U+FFFD (decision, plan M3.6-7).  Advances p. */
      inline uint32_t u16_next_cp(const wchar_t *&p, const wchar_t *end)
      {
        uint32_t c = (uint32_t)*p++;
        if (c >= 0xD800 && c <= 0xDBFF && p != end) {
          uint32_t lo = (uint32_t)*p;
          if (lo >= 0xDC00 && lo <= 0xDFFF) {
            ++p;
            return 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
          }
        }
        if (c >= 0xD800 && c <= 0xDFFF)
          return 0xFFFD; /* lone surrogate */
        return c;
      }

      /* Decode one code point from UTF-8 [p, end).  Malformed input
       * yields U+FFFD and consumes a single byte (defensive only: the
       * engine renders valid UTF-8). */
      inline uint32_t u8_next_cp(const char *&p, const char *end)
      {
        unsigned char c = (unsigned char)*p++;
        if (c < 0x80)
          return c;
        unsigned n;
        uint32_t cp;
        if ((c & 0xE0) == 0xC0) {
          n = 1;
          cp = c & 0x1F;
        } else if ((c & 0xF0) == 0xE0) {
          n = 2;
          cp = c & 0x0F;
        } else if ((c & 0xF8) == 0xF0) {
          n = 3;
          cp = c & 0x07;
        } else {
          return 0xFFFD;
        }
        if ((size_t)(end - p) < n)
          return 0xFFFD;
        for (unsigned i = 0; i < n; ++i) {
          unsigned char cc = (unsigned char)*p;
          if ((cc & 0xC0) != 0x80)
            return 0xFFFD;
          cp = (cp << 6) | (cc & 0x3F);
          ++p;
        }
        if ((n == 1 && cp < 0x80) || (n == 2 && cp < 0x800) ||
            (n == 3 && cp < 0x10000) || cp > 0x10FFFF)
          return 0xFFFD; /* overlong or out of range */
        return cp;
      }

      /* Emit one code point as UTF-8 through the sink. */
      template <typename Emit>
      inline void utf8_emit(Emit &&emit, uint32_t cp)
      {
        if (cp < 0x80) {
          emit((char)cp);
        } else if (cp < 0x800) {
          emit((char)(0xC0 | (cp >> 6)));
          emit((char)(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
          emit((char)(0xE0 | (cp >> 12)));
          emit((char)(0x80 | ((cp >> 6) & 0x3F)));
          emit((char)(0x80 | (cp & 0x3F)));
        } else {
          emit((char)(0xF0 | (cp >> 18)));
          emit((char)(0x80 | ((cp >> 12) & 0x3F)));
          emit((char)(0x80 | ((cp >> 6) & 0x3F)));
          emit((char)(0x80 | (cp & 0x3F)));
        }
      }

      /* Transcode n wchar units into dst (which needs room for 3*n+1
       * bytes — every unit is at most 3 bytes: BMP <= 3, a surrogate
       * pair is 2 units -> 4 bytes, a lone surrogate -> U+FFFD).
       * Always terminates; returns bytes written, NUL excluded. */
      inline size_t u16_to_u8(const wchar_t *ws, size_t n, char *dst)
      {
        const wchar_t *p = ws;
        const wchar_t *end = ws + n;
        char *q = dst;
        while (p != end) {
          uint32_t cp = u16_next_cp(p, end);
          utf8_emit([&q](char c) { *q++ = c; }, cp);
        }
        *q = '\0';
        return (size_t)(q - dst);
      }

      /* Transcode UTF-8 [s, s+n) to UTF-16.  Returns the wchar count
       * the whole input needs.  When dst is non-null, at most `cap`
       * units are stored — with `nul` set the store ends with a
       * terminator (at most cap-1 data units); a surrogate pair may be
       * cut after its high unit, matching wchar-granular truncation
       * (plan M3.6-5). */
      inline size_t
      u8_to_u16(const char *s, size_t n, wchar_t *dst, size_t cap, bool nul)
      {
        const char *p = s;
        const char *end = s + n;
        size_t need = 0;
        size_t wrote = 0;
        size_t budget = dst ? (nul ? (cap ? cap - 1 : 0) : cap) : 0;
        while (p != end) {
          uint32_t cp = u8_next_cp(p, end);
          if (cp < 0x10000) {
            ++need;
            if (dst && wrote < budget)
              dst[wrote++] = (wchar_t)cp;
          } else {
            need += 2;
            if (dst && wrote + 1 < budget) {
              cp -= 0x10000;
              dst[wrote++] = (wchar_t)(0xD800 | (cp >> 10));
              dst[wrote++] = (wchar_t)(0xDC00 | (cp & 0x3FF));
            } else if (dst && wrote < budget) {
              /* one slot left: the pair is cut after its high unit */
              dst[wrote++] = (wchar_t)(0xD800 | ((cp - 0x10000) >> 10));
            }
          }
        }
        if (dst && nul && wrote < cap)
          dst[wrote] = L'\0';
        return need;
      }

      /* ------------------- format pre-translator ------------------- */

      /* Wide -> narrow format translation (plan M3.2).  Returns the
       * full required size including the terminator.  When out/cap
       * are given, at most cap-1 bytes plus a terminator are stored
       * (clip-and-count: one pass, safe on short buffers).  %n is
       * copied verbatim (musl semantics: always enabled).
       *
       * Buffer bound: 3*wcslen(fmt)+16 always suffices (each wchar is
       * at most 3 UTF-8 bytes, a rewrite grows a directive by at most
       * one character). */
      inline size_t wfmt_translate(const wchar_t *fmt,
                                   bool scanf_mode,
                                   bool legacy_wide,
                                   char *out,
                                   size_t cap)
      {
        size_t pos = 0;
        bool clip = !out || cap == 0;

        auto put = [&](char c) {
          if (!clip && pos + 1 < cap)
            out[pos] = c;
          ++pos;
        };
        /* transcode and emit a wide substring */
        auto putw = [&](const wchar_t *a, const wchar_t *b) {
          while (a != b)
            utf8_emit(put, u16_next_cp(a, b));
        };

        const wchar_t *p = fmt;
        while (*p) {
          if (*p != L'%') {
            const wchar_t *a = p;
            while (*p && *p != L'%')
              ++p;
            putw(a, p); /* text run */
            continue;
          }

          /* directive scan (flags/width/precision/modifiers grammar —
           * mirrors the engine's directive parser) */
          const wchar_t *start = p;
          ++p; /* past '%' */
          if (*p == L'%') { /* %% */
            putw(start, p + 1);
            ++p;
            continue;
          }

          /* positional argument: digits immediately followed by '$' */
          if (*p >= L'0' && *p <= L'9') {
            const wchar_t *q = p;
            while (*q >= L'0' && *q <= L'9')
              ++q;
            if (*q == L'$')
              p = q + 1;
          }

          /* flags */
          while (*p == L'#' || *p == L'0' || *p == L'-' || *p == L' ' ||
                 *p == L'+' || *p == L'\'')
            ++p;

          /* width (fixed or '*' forms, incl. '*m$') */
          if (*p == L'*') {
            ++p;
            if (*p >= L'0' && *p <= L'9') {
              const wchar_t *q = p;
              while (*q >= L'0' && *q <= L'9')
                ++q;
              if (*q == L'$')
                p = q + 1;
            }
          } else {
            while (*p >= L'0' && *p <= L'9')
              ++p;
          }

          /* precision */
          if (*p == L'.') {
            ++p;
            if (*p == L'*') {
              ++p;
              if (*p >= L'0' && *p <= L'9') {
                const wchar_t *q = p;
                while (*q >= L'0' && *q <= L'9')
                  ++q;
                if (*q == L'$')
                  p = q + 1;
              }
            } else {
              while (*p >= L'0' && *p <= L'9')
                ++p;
            }
          }

          /* length modifiers (kept out of the prefix so a rewrite can
           * replace them); 'w' tracked like the M2 lexer */
          const wchar_t *mods = p;
          for (;;) {
            if (p[0] == L'h' && p[1] == L'h') {
              p += 2;
              continue;
            }
            if (p[0] == L'l' && p[1] == L'l') {
              p += 2;
              continue;
            }
            if (*p == L'h' || *p == L'l' || *p == L'j' || *p == L'z' ||
                *p == L't' || *p == L'L' || *p == L'w') {
              ++p;
              continue;
            }
            break;
          }
          const wchar_t *mods_end = p;
          wchar_t t = *p;

          /* scanset body for %[ (scanf): a ']' first after '[' is
           * literal, the body runs to the closing ']' */
          if (t == L'[') {
            ++p;
            if (*p == L']')
              ++p;
            while (*p && *p != L']')
              ++p;
            if (*p == L']')
              ++p;
          }

          if (t == 0) { /* trailing malformed directive */
            putw(start, p);
            continue;
          }

          /* s/c-family rewrite: only when a wide convention is active.
           * In ISO printf mode everything stays verbatim (= musl).
           * '[' is a scanf conversion only. */
          bool conv_family = t == L's' || t == L'c' || t == L'S' ||
                             t == L'C' || (t == L'[' && scanf_mode);
          if (conv_family && (scanf_mode || legacy_wide)) {
            bool narrow = false;
            bool wide_mod = false;
            for (const wchar_t *m = mods; m != mods_end; ++m) {
              if (*m == L'h')
                narrow = true; /* hh behaves like h for s/c */
              else if (*m == L'l' || *m == L'w')
                wide_mod = true;
            }
            /* %S/%C are the narrow spellings of the wide functions;
             * everything else defaults to wide under a convention */
            bool out_wide = (t == L'S' || t == L'C') ? wide_mod
                                                     : (wide_mod || !narrow);

            put(L'%');
            putw(start + 1, mods); /* positional/flags/width/precision */
            if (out_wide)
              put(L'l');
            if (t == L'[') {
              /* the scanset (brackets + body) rides verbatim */
              putw(mods_end, p);
            } else {
              put((char)(t == L'S' ? 's' : (t == L'C' ? 'c' : t)));
              ++p; /* past the conversion character */
            }
            continue;
          }

          /* everything else — %d/%f/%p/%n-like modifiers, unknown or
           * MS-specific directives — goes to the engine verbatim */
          putw(start, p);
        }

        if (!clip)
          out[pos < cap ? pos : cap - 1] = '\0';
        return pos + 1;
      }

      /* -------------------- shell-side conveniences ----------------- */

      /* Translated-format holder: stack buffer for common formats,
       * heap fallback for the rare long one.  ok=false only on OOM. */
      struct wfmt_holder
      {
        char stackbuf[512];
        char *nfmt;
        bool heap;
        bool ok;

        wfmt_holder(const wchar_t *fmt, bool scanf_mode, bool legacy_wide)
            : nfmt(stackbuf), heap(false), ok(false)
        {
          size_t bound = 3 * wcslen(fmt) + 16;
          if (bound > sizeof stackbuf) {
            nfmt = (char *)malloc(bound);
            if (!nfmt)
              return;
            heap = true;
          }
          wfmt_translate(fmt, scanf_mode, legacy_wide, nfmt, bound);
          ok = true;
        }

        ~wfmt_holder()
        {
          if (heap)
            free(nfmt);
        }

        wfmt_holder(const wfmt_holder &) = delete;
        wfmt_holder &operator=(const wfmt_holder &) = delete;
      };

      /* Full UTF-8 render of one wide printf call (two-pass
       * measure/render with the same va_list — M2 precedent).  ok=false
       * covers engine failure and scratch OOM. */
      struct wide_render
      {
        char stackbuf[512];
        char *buf;
        size_t bytes; /* rendered length, terminator excluded */
        bool heap;
        bool ok;

        wide_render(const char *nfmt, va_list ap)
            : buf(stackbuf), bytes(0), heap(false), ok(false)
        {
          int need = musl::vsnprintf(nullptr, 0, nfmt, ap);
          if (need < 0)
            return;
          if ((size_t)need + 1 > sizeof stackbuf) {
            buf = (char *)malloc((size_t)need + 1);
            if (!buf)
              return;
            heap = true;
          }
          int r = musl::vsnprintf(buf, (size_t)need + 1, nfmt, ap);
          if (r < 0)
            return;
          bytes = (size_t)r;
          ok = true;
        }

        ~wide_render()
        {
          if (heap)
            free(buf);
        }

        wide_render(const wide_render &) = delete;
        wide_render &operator=(const wide_render &) = delete;
      };
    } // namespace shell
  } // namespace i
} // namespace mingw_thunk
