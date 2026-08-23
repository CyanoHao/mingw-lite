// musl strtox wrapper (upstream src/stdlib/strtod.c shape, verbatim port):
// a synthetic memory FILE over the NUL-terminated string, __floatscan with
// pok=1, and the shcnt offset folded back into *endptr.
//
// prec mapping (musl upstream): strtof=0 (FLT), strtod=1 (DBL), strtold=2
// (LDBL).  On this target (i686 mingw) long double is the 80-bit x87 type,
// the same ABI ucrtbase's strtold returns in st(0) — wine probe 2026-09-27
// anchors native strtold as a true 80-bit parse (strtold("0.1") bit-identical
// to the 0.1L literal; "1e310" finite; "1e5000" inf + errno 34).

#include "../internal/floatscan.h"
#include "../internal/shgetc.h"

#include <stddef.h>

namespace mingw_thunk
{
  namespace musl
  {
    static long double strtox(const char *s, char **p, int prec)
    {
      FILE f = {};
      f.buf = (unsigned char *)s;
      f.rpos = (unsigned char *)s;
      f.rend = (unsigned char *)-1;
      f.lock = -1;
      shlim(&f, 0);
      long double y = __floatscan(&f, prec, 1);
      off_t cnt = shcnt(&f);
      if (p)
        *p = (char *)s + cnt;
      return y;
    }

    double strtod(const char *s, char **p)
    {
      return strtox(s, p, 1);
    }

    float strtof(const char *s, char **p)
    {
      return strtox(s, p, 0);
    }

    long double strtold(const char *s, char **p)
    {
      return strtox(s, p, 2);
    }
  } // namespace musl
} // namespace mingw_thunk
