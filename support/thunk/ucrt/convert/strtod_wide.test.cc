#include <catch_amalgamated.hpp>

#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

  // Faces the mingw headers omit on i686
  extern "C" long double __cdecl _wcstold_l(const wchar_t *, wchar_t **,
                                           _locale_t);
  extern "C" float __cdecl _wcstof_l(const wchar_t *, wchar_t **,
                                     _locale_t);

static double wrt(const wchar_t *s, long *end_off, int *err)
{
  wchar_t *end = nullptr;
  errno = 0;
  double v = wcstod(s, &end);
  *end_off = long(end - s);
  *err = errno;
  return v;
}

TEST_CASE("wcstod wide anchors")
{
  // Probe B (wine ucrtbase).
  long off;
  int err;

  // CJK tail: value parsed, endptr at the CJK unit (D4 view: consumed
  // ASCII bytes == consumed wide units)
  REQUIRE(wrt(L"3.14\x4f60", &off, &err) == 3.14);
  REQUIRE(off == 4);
  REQUIRE(err == 0);

  // ASCII whitespace skipped
  REQUIRE(wrt(L"  2.5", &off, &err) == 2.5);
  REQUIRE(off == 5);

  // hex / inf / nan through the wide view
  REQUIRE(wrt(L"0x1.8p3", &off, &err) == 12.0);
  REQUIRE(off == 7);
  REQUIRE(wrt(L"inf", &off, &err) == HUGE_VAL);
  REQUIRE(off == 3);
  REQUIRE(wrt(L"INF", &off, &err) == HUGE_VAL);
  double nan_v = wrt(L"nan(0x10)", &off, &err);
  REQUIRE(isnan(nan_v));
  REQUIRE(off == 9); // narrow shape: payload consumed (native wide stops
                     // at '(' — documented divergence, plan-3 §3.7)

  // non-ASCII leading: not whitespace for the engine (D13 divergence —
  // native skips U+3000/U+00A0)
  REQUIRE(wrt(L"\x3000" L"3.14", &off, &err) == 0.0);
  REQUIRE(off == 0);

  // pure CJK / surrogate pair prefix -> miss at start
  REQUIRE(wrt(L"\x4f60\x597d", &off, &err) == 0.0);
  REQUIRE(off == 0);
  REQUIRE(wrt(L"\xd83d\xde00", &off, &err) == 0.0);
  REQUIRE(off == 0);

  // lone surrogate tail: parse unaffected, endptr at the surrogate
  REQUIRE(wrt(L"3.14\xd800", &off, &err) == 3.14);
  REQUIRE(off == 4);

  // overflow sets ERANGE (wide side too)
  REQUIRE(wrt(L"1e999", &off, &err) == HUGE_VAL);
  REQUIRE(err == ERANGE);

  // no-endptr call
  REQUIRE(wcstod(L"1.5", nullptr) == 1.5);

  // null: graceful 0 + EINVAL (native wide r2 anchor)
  errno = 0;
  REQUIRE(wcstod(nullptr, nullptr) == 0.0);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("wcstof/wcstold and _wtof")
{
  float f = wcstof(L"3.14", nullptr);
  REQUIRE(f == 3.14f);

  long double ld = wcstold(L"0.1", nullptr);
  REQUIRE(memcmp(&ld, "\xcd\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xfb\x3f", 10) == 0);
  ld = wcstold(L"1e310", nullptr);
  REQUIRE(isfinite(ld));

  // CJK tail (_wtof = wcstod without endptr)
  REQUIRE(_wtof(L"2.5\x4f60") == 2.5);
  errno = 0;
  REQUIRE(_wtof(nullptr) == 0.0); // r12: graceful + EINVAL
  REQUIRE(errno == EINVAL);

  // _l shells ignore the locale argument (null safe)
  wchar_t *end = nullptr;
  REQUIRE(_wcstod_l(L"3.14\x4f60", &end, nullptr) == 3.14);
  REQUIRE(long(end - L"3.14\x4f60") == 4);
  REQUIRE(_wcstof_l(L"0.1", &end, nullptr) == 0.1f);
  REQUIRE(_wcstold_l(L"1e310", &end, nullptr) > 1e300L);
  REQUIRE(_wtof_l(L"2.5", nullptr) == 2.5);
}
