#include <catch_amalgamated.hpp>

#include <errno.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// C++ TUs on this toolchain always get __USE_MINGW_STRTOX=1 (libstdc++'s
// os_defines.h undefs __USE_MINGW_ANSI_STDIO and the C++11 branch of
// _mingw.h re-enables it), which header-inlines the narrow strtod/strtof
// names to libmingwex's __mingw_strtod family.  The overlay's own thunks
// are therefore driven here through the _l faces (plain dllimport
// declarations that bind to the overlay and delegate to the exact same
// thunk bodies — plan-3 §3.7).
extern "C" long double __cdecl _strtold_l(const char *, char **, _locale_t);

static double rt(const char *s, long *end_off, int *err)
{
  char *end = nullptr;
  errno = 0;
  double v = _strtod_l(s, &end, nullptr);
  *end_off = end ? long(end - s) : -1;
  *err = errno;
  return v;
}

TEST_CASE("strtod narrow anchors")
{
  // Probe A (wine ucrtbase) — all shapes matched by the musl engine.
  long off;
  int err;

  REQUIRE(rt("3.14abc", &off, &err) == 3.14);
  REQUIRE(off == 4);
  REQUIRE(err == 0);

  REQUIRE(rt("0x1.8p3", &off, &err) == 12.0);
  REQUIRE(off == 7);
  REQUIRE(rt("0x.8p1", &off, &err) == 1.0);
  REQUIRE(off == 6);
  REQUIRE(rt("0xg", &off, &err) == 0.0); // consumed "0", stops at 'x'
  REQUIRE(off == 1);
  REQUIRE(rt("+0x10", &off, &err) == 16.0); // hex without p accepted
  REQUIRE(off == 5);

  REQUIRE(rt("inf", &off, &err) == HUGE_VAL);
  REQUIRE(off == 3);
  REQUIRE(rt("INF", &off, &err) == HUGE_VAL);
  REQUIRE(rt("infinity", &off, &err) == HUGE_VAL);
  REQUIRE(off == 8);
  REQUIRE(rt("-inF", &off, &err) == -HUGE_VAL);
  REQUIRE(off == 4);

  // NaN family: value NaN, payload consumed to the closing paren.
  double nan_v = rt("NAN(0x10)", &off, &err);
  REQUIRE(isnan(nan_v));
  REQUIRE(off == 9);
  nan_v = rt("nan(_x9)", &off, &err);
  REQUIRE(isnan(nan_v));
  REQUIRE(off == 8);
  nan_v = rt("nan(0x10)tail", &off, &err);
  REQUIRE(isnan(nan_v));
  REQUIRE(off == 9);
  nan_v = rt("-nan", &off, &err);
  REQUIRE(isnan(nan_v));
  REQUIRE(signbit(nan_v)); // native prints -nan(ind): sign bit set
  REQUIRE(off == 4);

  // ERANGE: overflow -> ±inf; inexact underflow -> 0/subnormal.
  REQUIRE(rt("1e999", &off, &err) == HUGE_VAL);
  REQUIRE(err == ERANGE);
  REQUIRE(rt("-1e999", &off, &err) == -HUGE_VAL);
  REQUIRE(err == ERANGE);
  REQUIRE(rt("1e-400", &off, &err) == 0.0);
  REQUIRE(err == ERANGE);
  double d5 = rt("5e-324", &off, &err); // inexact subnormal
  REQUIRE(d5 == DBL_TRUE_MIN);
  REQUIRE(err == ERANGE);
  // 17-digit near-exact decimal of 2^-1074: both wine native and the
  // (now correctly calibrated) engine report errno 0 — the rounding to
  // the target precision loses nothing the engine tracks (probe3 +
  // engine fix, plan-3 §3.7).
  double dx = rt("4.9406564584124654e-324", &off, &err);
  REQUIRE(dx == DBL_TRUE_MIN);
  REQUIRE(err == 0);
  REQUIRE(rt("2.2250738585072014e-308", &off, &err) == DBL_MIN);
  REQUIRE(err == 0);

  // Whitespace / edge forms
  REQUIRE(rt("  +2.", &off, &err) == 2.0);
  REQUIRE(off == 5);
  REQUIRE(rt(".5", &off, &err) == 0.5);
  REQUIRE(off == 2);
  REQUIRE(rt("e5", &off, &err) == 0.0);
  REQUIRE(off == 0);
  REQUIRE(err == 0); // miss is not an error (native leaves errno alone)
  REQUIRE(rt("abc", &off, &err) == 0.0);
  REQUIRE(off == 0);
  REQUIRE(err == 0);
  REQUIRE(rt("1e+", &off, &err) == 1.0); // bare exponent sign is unread
  REQUIRE(off == 1);
  REQUIRE(rt("1e", &off, &err) == 1.0);
  REQUIRE(off == 1);
  REQUIRE(rt("007.5", &off, &err) == 7.5);
  REQUIRE(off == 5);
  REQUIRE(rt("1_000", &off, &err) == 1.0); // no digit grouping
  REQUIRE(off == 1);
  double mz = rt("-.0", &off, &err);
  REQUIRE(mz == 0.0);
  REQUIRE(signbit(mz));
  REQUIRE(off == 3);

  // null string: graceful 0 + EINVAL (native narrow crashes; wide native
  // and atof natives are graceful — family shape, plan-3 §3.7)
  errno = 0;
  REQUIRE(_strtod_l(nullptr, nullptr, nullptr) == 0.0);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("strtof/strtold precision")
{
  long off;
  int err;

  // float: correctly rounded to FLT (probe A2)
  float f = _strtof_l("1.0000000596046448", nullptr, nullptr);
  REQUIRE(f == 1.00000012f);
  f = _strtof_l("0.1", nullptr, nullptr);
  REQUIRE(f == 0.1f);
  REQUIRE(memcmp(&f, "\xcd\xcc\xcc\x3d", 4) == 0);
  errno = 0;
  f = _strtof_l("1e999", nullptr, nullptr);
  REQUIRE(isinf(f));
  REQUIRE(errno == ERANGE);
  errno = 0;
  f = _strtof_l("1e-999", nullptr, nullptr);
  REQUIRE(f == 0.0f);
  REQUIRE(errno == ERANGE);

  // long double: true 80-bit parse (wine anchors: 0.1L literal bits,
  // 1e310 finite, 1e5000 inf + ERANGE)
  long double ld = _strtold_l("0.1", nullptr, nullptr);
  REQUIRE(memcmp(&ld, "\xcd\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xfb\x3f", 10) == 0);
  ld = _strtold_l("1e310", nullptr, nullptr);
  REQUIRE(isfinite(ld));
  REQUIRE((double)ld == HUGE_VAL);
  errno = 0;
  ld = _strtold_l("1e5000", nullptr, nullptr);
  REQUIRE(isinf(ld));
  REQUIRE(errno == ERANGE);
  ld = _strtold_l("1.00000000000000000001", nullptr, nullptr);
  REQUIRE(ld == 1.0L); // 80-bit rounds away the 1e-20

  // the _l shells delegate to the same thunk bodies as the base names
  char *end = nullptr;
  REQUIRE(_strtod_l("3.14abc", &end, nullptr) == 3.14);
  REQUIRE(long(end - "3.14abc") == 4);
}

TEST_CASE("atof family")
{
  // r11 anchor shape (null -> 0 + EINVAL), driven through the _l face
  // (atof itself is header-captured in C++ TUs; _atof_l delegates to the
  // same thunk)
  errno = 0;
  REQUIRE(_atof_l(nullptr, nullptr) == 0.0);
  REQUIRE(errno == EINVAL);

  REQUIRE(_atof_l("3.5", nullptr) == 3.5);
  REQUIRE(_atof_l("  -2.5e2", nullptr) == -250.0);
  REQUIRE(_atof_l("", nullptr) == 0.0);
  REQUIRE(_atof_l("2.25", nullptr) == 2.25);
}

TEST_CASE("LC_NUMERIC pollution immunity")
{
  // Native control group (probe E): after setlocale(LC_NUMERIC, "de-DE")
  // native strtod("1.5") truncates to 1 and _gcvt renders "1,5".  Our
  // setlocale is the self-report overlay (no native state is switched),
  // and the engine is frozen-C by construction — assert both.
  const char *r = setlocale(LC_NUMERIC, "de-DE");
  REQUIRE(r != nullptr); // overlay self-report (never switches anything)

  errno = 0;
  REQUIRE(_strtod_l("1.5", nullptr, nullptr) == 1.5);
  REQUIRE(errno == 0);

  char buf[32];
  REQUIRE(_gcvt(1.5, 6, buf) == buf);
  REQUIRE(strcmp(buf, "1.5") == 0);

  REQUIRE(_atof_l("2.5", nullptr) == 2.5);
  REQUIRE(_wtof(L"2.5") == 2.5);
}
