#include "../internal/stdio_impl.h"

#include <thunk/u8crt/musl.h>

#include <string.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      size_t string_read(FILE *f, unsigned char *buf, size_t len)
      {
        char *src = (char *)f->cookie;
        size_t k = len + 256;
        char *end = (char *)memchr(src, 0, k);
        if (end)
          k = (size_t)(end - src);
        if (k < len)
          len = k;
        memcpy(buf, src, len);
        f->rpos = (unsigned char *)src + len;
        f->rend = (unsigned char *)src + k;
        f->cookie = src + k;
        return len;
      }
    } // namespace

    int vsscanf(const char *s, const char *fmt, va_list ap)
    {
      FILE f = {};
      f.read = &string_read;
      f.buf = (unsigned char *)s;
      f.cookie = (char *)s;
      f.lock = -1;
      return vfscanf(&f, fmt, ap);
    }
  } // namespace musl
} // namespace mingw_thunk
