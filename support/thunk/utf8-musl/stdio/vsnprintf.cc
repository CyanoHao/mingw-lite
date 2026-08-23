#include "../internal/stdio_impl.h"

#include <thunk/u8crt/musl.h>

#include <string.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      struct cookie
      {
        char *s;
        size_t n;
      };

      size_t sn_write(FILE *f, const unsigned char *s, size_t l)
      {
        cookie *c = (cookie *)f->cookie;
        size_t k = (size_t)(f->wpos - f->wbase);
        if (k > c->n)
          k = c->n;
        if (k) {
          memcpy(c->s, f->wbase, k);
          c->s += k;
          c->n -= k;
        }
        k = l;
        if (k > c->n)
          k = c->n;
        if (k) {
          memcpy(c->s, s, k);
          c->s += k;
          c->n -= k;
        }
        *c->s = 0;
        f->wpos = f->wbase = f->buf;
        /* pretend to succeed, even if we discarded extra data */
        return l;
      }
    } // namespace

    int vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
    {
      unsigned char buf[1];
      char dummy[1];
      cookie c = {n ? s : dummy, n ? n - 1 : 0};
      FILE f = {};
      f.write = &sn_write;
      f.buf = buf;
      f.lock = -1;
      f.lbf = EOF;
      f.cookie = &c;

      *c.s = 0;
      return vfprintf(&f, fmt, ap);
    }
  } // namespace musl
} // namespace mingw_thunk
