#include "../internal/stdio_impl.h"

#include <thunk/u8crt/musl.h>

#include <stdio.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      /* native revert imports (def aliases): FILE-family natives must
       * go through __ms_ so they can never be re-intercepted through
       * our own overlay, keeping the layering of utf8-musl intact
       * (the direct-native precedent, _get_osfhandle, is fd-based) */
      extern "C" {
      __attribute__((dllimport)) size_t __cdecl __ms_fwrite(
          const void *, size_t, size_t, ::FILE *);
      __attribute__((dllimport)) int __cdecl __ms_ungetc(int, ::FILE *);
      __attribute__((dllimport)) void __cdecl __ms__lock_file(::FILE *);
      __attribute__((dllimport)) void __cdecl __ms__unlock_file(::FILE *);
      __attribute__((dllimport)) int __cdecl __ms__fgetc_nolock(::FILE *);
      }

      /* ---- write side ------------------------------------------------ */

      /* mirrors the __stdio_write contract: pending [wbase, wpos) first,
       * then the new chunk — both go back to the SAME native FILE so
       * its buffering, position and interleaving stay authoritative */
      size_t bridge_write(FILE *f, const unsigned char *s, size_t l)
      {
        ::FILE *native = (::FILE *)f->cookie;
        size_t pending = (size_t)(f->wpos - f->wbase);
        if (pending && __ms_fwrite(f->wbase, 1, pending, native) != pending) {
          f->wpos = f->wbase = f->wend = 0;
          f->flags |= F_ERR;
          return 0;
        }
        if (l && __ms_fwrite(s, 1, l, native) != l) {
          f->wpos = f->wbase = f->wend = 0;
          f->flags |= F_ERR;
          return 0;
        }
        f->wpos = f->wbase = f->buf;
        return l;
      }

      /* ---- read side ------------------------------------------------- */

      constexpr size_t kKeep = (size_t)UNGET; /* history below f->buf   */
      constexpr size_t kWnd = 64;             /* append region          */

      struct read_bridge
      {
        unsigned char window[kKeep + kWnd];
        ::FILE *native;
        size_t fill; /* logical stream position, as an index into window */
      };

      /* Zero-lookahead read fn: pulls exactly the requested bytes from
       * the native FILE, appends them to the ring (compacting so that
       * shunget retreats stay inside the window) and publishes
       * rpos == append end, rend == rpos.  Because __uflow only fires
       * at rpos == shend == fill (retreated bytes are re-delivered by
       * the shgetc fast path instead), the unconsumed lookahead at any
       * teardown is at most the one byte of a trailing shunget. */
      size_t bridge_read(FILE *f, unsigned char *buf, size_t len)
      {
        read_bridge *b = (read_bridge *)f->cookie;

        if (b->fill + len > sizeof b->window) {
          /* compact: keep the newest kKeep bytes as retreat history */
          memmove(b->window, b->window + b->fill - kKeep, kKeep);
          b->fill = kKeep;
        }

        size_t got = 0;
        while (got < len) {
          int c = __ms__fgetc_nolock(b->native);
          if (c == EOF) {
            f->flags |= F_EOF;
            break;
          }
          b->window[b->fill++] = (unsigned char)c;
          buf[got++] = (unsigned char)c;
        }

        f->rpos = b->window + b->fill;
        f->rend = f->rpos;
        return got;
      }
    } // namespace

    int vfprintf_to_native(void *native_fp, const char *fmt, va_list ap)
    {
      unsigned char buf[256];
      FILE f = {};
      f.write = &bridge_write;
      f.buf = buf;
      f.buf_size = sizeof buf;
      f.lock = -1;
      f.lbf = '\n';
      f.cookie = native_fp;

      int r = vfprintf(&f, fmt, ap);
      if (f.wpos != f.wbase) /* trailing partial line */
        f.write(&f, 0, 0);
      return r;
    }

    int vfscanf_from_native(void *native_fp, const char *fmt, va_list ap)
    {
      read_bridge b = {};
      b.native = (::FILE *)native_fp;
      b.fill = kKeep;

      FILE f = {};
      f.read = &bridge_read;
      f.buf = b.window + kKeep; /* makes rpos[-1 .. -8] valid history */
      f.buf_size = kWnd;
      f.lock = -1;
      f.cookie = &b;

      __ms__lock_file(b.native);
      int r = vfscanf(&f, fmt, ap);

      /* push the unconsumed lookahead back into the native FILE so the
       * native stream position equals the scanf logical position; by
       * construction this is at most one byte (defensive loop for the
       * impossible case) */
      if (f.rpos) {
        size_t ridx = (size_t)(f.rpos - b.window);
        for (size_t i = b.fill; i > ridx; i--)
          __ms_ungetc(b.window[i - 1], b.native);
      }
      __ms__unlock_file(b.native);
      return r;
    }
  } // namespace musl
} // namespace mingw_thunk
