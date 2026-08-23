#include <catch_amalgamated.hpp>

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include <string>

// Memory-channel tests (musl layer): vsnprintf/vsscanf drive the M0
// engine through the string-file shapes ported from musl upstream.
#include <thunk/u8crt/musl.h>

namespace
{
  namespace musl = mingw_thunk::musl;

  int render(char *out, size_t n, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = musl::vsnprintf(out, n, f, ap);
    va_end(ap);
    return r;
  }

  std::string fmt(size_t n, const char *f, ...)
  {
    std::string out(n > 0 ? n : 1, '\0');
    va_list ap;
    va_start(ap, f);
    musl::vsnprintf(n ? &out[0] : nullptr, n, f, ap);
    va_end(ap);
    return out.c_str();
  }

  int scan(const char *s, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = musl::vsscanf(s, f, ap);
    va_end(ap);
    return r;
  }
} // namespace

TEST_CASE("vsnprintf memory channel formatting")
{
  char buf[128];

  REQUIRE(render(buf, sizeof buf, "%s=%d", "key", 42) == 6);
  REQUIRE(strcmp(buf, "key=42") == 0);

  REQUIRE(render(buf, sizeof buf, "%.2f|%08x|%5.3s", 1.5, 0xabc, "hello") ==
          19);
  REQUIRE(strcmp(buf, "1.50|00000abc|  hel") == 0);

  // UTF-8 passthrough and %ls encoding (M0 engine, surrogate pairs)
  REQUIRE(fmt(128, "%s!", "你好") == "\xe4\xbd\xa0\xe5\xa5\xbd!");
  REQUIRE(fmt(128, "%ls", L"你好") == "\xe4\xbd\xa0\xe5\xa5\xbd");
  REQUIRE(fmt(128, "%ls", L"\U0001F44D") == "\xf0\x9f\x91\x8d");

  // zero padding, signedness, long long
  REQUIRE(fmt(128, "%+05d|%lld", 42, -9223372036854775807LL) ==
          "+0042|-9223372036854775807");
}

TEST_CASE("vsnprintf truncation semantics")
{
  char small[4];
  int r = render(small, sizeof small, "%s", "abcdef");
  REQUIRE(r == 6); // C11: the return is the required length
  REQUIRE(strcmp(small, "abc") == 0);

  // n == 1: only the terminator fits
  r = render(small, 1, "%d", 7);
  REQUIRE(r == 1);
  REQUIRE(small[0] == '\0');

  // n == 0: nothing is written (not even a NUL)
  REQUIRE(render(nullptr, 0, "%d", 123) == 3);
}

TEST_CASE("vsscanf memory channel conversions")
{
  int a, b;
  double d;
  unsigned u;
  char s[16];

  REQUIRE(scan("10 -20", "%d %d", &a, &b) == 2);
  REQUIRE(a == 10);
  REQUIRE(b == -20);

  REQUIRE(scan("3.25e1", "%lf", &d) == 1);
  REQUIRE(d == 32.5);

  REQUIRE(scan("ff", "%x", &u) == 1);
  REQUIRE(u == 0xff);

  REQUIRE(scan("hello world", "%s", s) == 1);
  REQUIRE(strcmp(s, "hello") == 0);

  // width-bounded scans and %n position (%n does not count as a
  // conversion per C11)
  REQUIRE(scan("123456", "%3d%n", &a, &b) == 1);
  REQUIRE(a == 123);
  REQUIRE(b == 3);

  // UTF-8 bytes pass through %s untouched
  REQUIRE(scan("\xe4\xbd\xa0\xe5\xa5\xbd rest", "%s", s) == 1);
  REQUIRE(strcmp(s, "\xe4\xbd\xa0\xe5\xa5\xbd") == 0);
}

TEST_CASE("vsscanf end-of-input semantics")
{
  int a, b;

  // input exhausted before the first conversion: EOF
  REQUIRE(scan("", "%d", &a) == -1);
  // literal-only exhaustion is also EOF
  REQUIRE(scan("", "x", &a) == -1);

  // matching failure keeps the converted prefix count
  REQUIRE(scan("42 abc", "%d %d", &a, &b) == 1);
  REQUIRE(a == 42);
}
