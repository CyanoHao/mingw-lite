#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <string>

// IAT-level shell tests (M2): the __stdio_common_* entry points are
// called directly through the overlay symbols, so every assertion
// below exercises the real routing (memory / stream / console
// channels, options handling, %n semantics, native passthrough for
// the SECURECRT va_list layout).
//
// The system libmingwex routes __mingw_* through its own pformat core
// (no __stdio_common_* references), so indirect plain-printf routing
// cannot be relied on here — direct symbol references are the
// documented mechanism (test/console.c precedent).

#ifndef _UCRT
extern "C"
{
  // _locale_t is opaque to the shells; a void pointer matches the ABI
  int __cdecl __stdio_common_vsprintf(
      unsigned __int64 options,
      char *str,
      size_t len,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vsnprintf_s(
      unsigned __int64 options,
      char *str,
      size_t size,
      size_t maxcount,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfprintf(
      unsigned __int64 options,
      FILE *stream,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfscanf(
      unsigned __int64 options,
      FILE *stream,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vsscanf(
      unsigned __int64 options,
      const char *input,
      size_t length,
      const char *format,
      const void *locale,
      va_list arglist);
}
#endif

namespace
{
  constexpr unsigned char kSentinel = 0xAA;

  int vs(unsigned __int64 options, char *str, size_t len, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsprintf(options, str, len, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int vsns(
      unsigned __int64 options,
      char *str,
      size_t size,
      size_t maxcount,
      const char *f,
      ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsnprintf_s(options, str, size, maxcount, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int vfs(unsigned __int64 options, FILE *fp, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfprintf(options, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int vfs_scan(unsigned __int64 options, FILE *fp, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfscanf(options, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int vss(unsigned __int64 options, const char *input, size_t length, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsscanf(options, input, length, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  // one va_list driven through two shell calls (measure + render),
  // as the legacy bounded path does internally
  void va_list_probe(char *out, size_t n, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int need = __stdio_common_vsprintf(0x2, nullptr, 0, f, nullptr, ap);
    int r = __stdio_common_vsprintf(0x2, out, n, f, nullptr, ap);
    va_end(ap);
    REQUIRE(need >= 0);
    REQUIRE(r == need);
  }

  std::string slurp(FILE *fp)
  {
    std::string out;
    rewind(fp);
    int c;
    while ((c = fgetc(fp)) != EOF)
      out.push_back((char)c);
    return out;
  }
} // namespace

TEST_CASE("shell memory channel: C11 snprintf semantics (options 0x2)")
{
  char buf[64];

  REQUIRE(vs(0x2, buf, sizeof buf, "%s=%d", "key", 42) == 6);
  REQUIRE(strcmp(buf, "key=42") == 0);

  // UTF-8 passthrough and %ls through the shell
  REQUIRE(vs(0x2, buf, sizeof buf, "%s!", "你好") == 7);
  REQUIRE(strcmp(buf, "\xe4\xbd\xa0\xe5\xa5\xbd!") == 0);
  REQUIRE(vs(0x2, buf, sizeof buf, "%ls", L"\U0001F44D") == 4);
  REQUIRE(strcmp(buf, "\xf0\x9f\x91\x8d") == 0);

  // truncation: required length returned, terminator always written
  char small[8];
  memset(small, kSentinel, sizeof small);
  REQUIRE(vs(0x2, small, 4, "%s", "helloworld") == 10);
  REQUIRE(memcmp(small, "hel", 3) == 0);
  REQUIRE(small[3] == '\0');
  REQUIRE((unsigned char)small[4] == kSentinel);

  // measurement (str == NULL, len == 0)
  REQUIRE(vs(0x2, nullptr, 0, "%d %s", 7, "x") == 3);
}

TEST_CASE("shell memory channel: legacy bounded _snprintf (options 0x1)")
{
  char buf[16];

  // fits: direct render, count bytes including terminator
  REQUIRE(vs(0x1, buf, sizeof buf, "%s", "hi") == 2);
  REQUIRE(strcmp(buf, "hi") == 0);

  // truncated: exactly `len` bytes, NO terminator, -1 — the byte past
  // the count must stay untouched
  char t[8];
  memset(t, kSentinel, sizeof t);
  REQUIRE(vs(0x1, t, 4, "%s", "helloworld") == -1);
  REQUIRE(memcmp(t, "hell", 4) == 0);
  REQUIRE((unsigned char)t[4] == kSentinel);

  // boundary: need == len is already truncation
  memset(t, kSentinel, sizeof t);
  REQUIRE(vs(0x1, t, 3, "%s", "abc") == -1);
  REQUIRE(memcmp(t, "abc", 3) == 0);
  REQUIRE((unsigned char)t[3] == kSentinel);

  // boundary: need == len - 1 still fits
  REQUIRE(vs(0x1, t, 4, "%s", "abc") == 3);
  REQUIRE(strcmp(t, "abc") == 0);

  // heap-scratch path (render larger than the 512-byte stack buffer)
  char big[12];
  memset(big, kSentinel, sizeof big);
  REQUIRE(vs(0x1, big, 10, "%600d", 5) == -1);
  REQUIRE(big[0] == ' ');
  REQUIRE(big[9] == ' ');
  REQUIRE((unsigned char)big[10] == kSentinel);
}

TEST_CASE("shell memory channel: legacy unbounded sprintf (0x1, len = -1)")
{
  char buf[64];

  REQUIRE(vs(0x1, buf, (size_t)-1, "%s-%d", "x", 9) == 3);
  REQUIRE(strcmp(buf, "x-9") == 0);

  // always NUL-terminated
  REQUIRE(vs(0x1, buf, (size_t)-1, "%s", "\xe4\xbd\xa0") == 3);
  REQUIRE(strcmp(buf, "\xe4\xbd\xa0") == 0);
}

TEST_CASE("shell memory channel: vsnprintf_s truncation matrix")
{
  char b[8];

  // _TRUNCATE: bound by the buffer, -1 on truncation, NUL kept
  memset(b, kSentinel, sizeof b);
  REQUIRE(vsns(0, b, 8, (size_t)-1, "%s", "helloworld") == -1);
  REQUIRE(strcmp(b, "hellowo") == 0);

  // _TRUNCATE: fits
  REQUIRE(vsns(0, b, 8, (size_t)-1, "%s", "hi") == 2);
  REQUIRE(strcmp(b, "hi") == 0);

  // tight maxcount < size: bound by maxcount+1
  memset(b, kSentinel, sizeof b);
  REQUIRE(vsns(0, b, 8, 3, "%s", "helloworld") == -1);
  REQUIRE(strcmp(b, "hel") == 0);
  REQUIRE((unsigned char)b[4] == kSentinel);

  // tight maxcount: exact fit (maxcount chars + NUL)
  REQUIRE(vsns(0, b, 8, 3, "%s", "abc") == 3);

  // roomy maxcount >= size: bound by the buffer
  memset(b, kSentinel, sizeof b);
  REQUIRE(vsns(0, b, 4, 10, "%s", "helloworld") == -1);
  REQUIRE(strcmp(b, "hel") == 0);
  REQUIRE((unsigned char)b[4] == kSentinel);

  // rejected shapes
  REQUIRE(vsns(0, nullptr, 8, (size_t)-1, "%d", 1) == -1);
  errno = 0;
  char ok[8];
  REQUIRE(vsns(0, ok, 0, (size_t)-1, "%d", 1) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("shell %n is always enabled (musl semantics)")
{
  char buf[32];
  int n = 123;

  // the engine implements %n unconditionally: no opt-in, no EINVAL,
  // no swallowed output, in every modifier/positional shape
  n = 0;
  REQUIRE(vs(0x2, buf, sizeof buf, "hi%n", &n) == 2);
  REQUIRE(n == 2);
  long l = 0;
  REQUIRE(vs(0x2, buf, sizeof buf, "%ln", &l) == 0);
  REQUIRE(l == 0);
  short h = 0;
  REQUIRE(vs(0x2, buf, sizeof buf, "abc%hn", &h) == 3);
  REQUIRE(h == 3);
  errno = 0;
  n = 5;
  REQUIRE(vs(0x2, buf, sizeof buf, "%1$n", &n) == 0);
  REQUIRE(n == 0);
  REQUIRE(errno == 0);
  REQUIRE(strcmp(buf, "") == 0);

  // the count-output switch is pinned on: get always reports enabled,
  // set reports the always-on previous state and closes nothing
  REQUIRE(_get_printf_count_output() == 1);
  REQUIRE(_set_printf_count_output(0) == 1);
  REQUIRE(_get_printf_count_output() == 1);
  n = 0;
  REQUIRE(vs(0x2, buf, sizeof buf, "%n", &n) == 0);
  REQUIRE(n == 0);
  REQUIRE(_set_printf_count_output(1) == 1);
}

TEST_CASE("shell three-digit exponents (options 0x10)")
{
  char buf[64];

  REQUIRE(vs(0x2, buf, sizeof buf, "%e", 12345.0) == 12);
  REQUIRE(strcmp(buf, "1.234500e+04") == 0);

  REQUIRE(vs(0x2 | 0x10, buf, sizeof buf, "%e", 12345.0) == 13);
  REQUIRE(strcmp(buf, "1.234500e+004") == 0);

  // the TLS state is restored for later calls
  REQUIRE(vs(0x2, buf, sizeof buf, "%e", 12345.0) == 12);
  REQUIRE(strcmp(buf, "1.234500e+04") == 0);
}

TEST_CASE("shell stream channel through a native FILE (tmpfile)")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);

  // write side: engine renders, bridge writes back, native writes
  // interleave in order
  REQUIRE(vfs(0, fp, "%s=%d", "k", 9) == 3);
  fputc('!', fp);
  REQUIRE(vfs(0, fp, "-%ls", L"你好") == 7);
  fputs("end", fp);
  fflush(fp);
  REQUIRE(slurp(fp) == "k=9!-\xe4\xbd\xa0\xe5\xa5\xbd"
                       "end");

  // read side: bridge pulls through the same stream
  FILE *fr = tmpfile();
  REQUIRE(fr);
  fputs("10 hello 2.5 X", fr);
  rewind(fr);
  int a;
  char s[16];
  double d;
  REQUIRE(vfs_scan(0, fr, "%d %s %lf", &a, s, &d) == 3);
  REQUIRE(a == 10);
  REQUIRE(strcmp(s, "hello") == 0);
  REQUIRE(d == 2.5);
  fclose(fr);

  fclose(fp);
}

TEST_CASE("shell vsscanf length semantics")
{
  int a;
  char c;

  // length = -1: NUL-terminated input
  REQUIRE(vss(0, "42 x", (size_t)-1, "%d %c", &a, &c) == 2);
  REQUIRE(a == 42);
  REQUIRE(c == 'x');

  // finite length without a NUL inside: read limit emulation
  REQUIRE(vss(0, "42 x", 2, "%d %c", &a, &c) == 1);
  REQUIRE(a == 42);

  // finite length with the NUL inside: same as terminated
  REQUIRE(vss(0, "42", 10, "%d", &a) == 1);
  REQUIRE(a == 42);

  // UTF-8 conversion through the memory read channel
  char b[16];
  REQUIRE(vss(0, "\xe4\xbd\xa0\xe5\xa5\xbd 7", (size_t)-1, "%s %d", b, &a) == 2);
  REQUIRE(strcmp(b, "\xe4\xbd\xa0\xe5\xa5\xbd") == 0);
  REQUIRE(a == 7);
}

TEST_CASE("shell SECURECRT (options 0x1) stays native")
{
  // the _s wrappers interleave a size_t after each buffer pointer;
  // the whole call is routed to the native implementation
  char sb[8];
  REQUIRE(vss(0x1, "abc", (size_t)-1, "%s", sb, (size_t)sizeof sb) == 1);
  REQUIRE(strcmp(sb, "abc") == 0);

  int i;
  REQUIRE(vss(0x1, "17", (size_t)-1, "%d", &i) == 1);
  REQUIRE(i == 17);
}

TEST_CASE("shell rejects null stream/buffer/format")
{
  char buf[8];
  errno = 0;
  REQUIRE(vs(0x2, nullptr, sizeof buf, "%d", 1) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(vfs(0, nullptr, "%d", 1) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(vfs(0, stdout, nullptr) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(vss(0, nullptr, (size_t)-1, "%d") == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("shell output is invariant under native locale state")
{
  char buf[64];

  // whatever the native locale machinery holds, the engine formats
  // with a '.' decimal separator and UTF-8 wide conversions
  setlocale(LC_ALL, "");
  setlocale(LC_NUMERIC, "C");

  REQUIRE(vs(0x2, buf, sizeof buf, "%f", 3.5) == 8);
  REQUIRE(strcmp(buf, "3.500000") == 0);
  REQUIRE(vs(0x2, buf, sizeof buf, "%ls", L"你好") == 6);
  REQUIRE(strcmp(buf, "\xe4\xbd\xa0\xe5\xa5\xbd") == 0);

  FILE *fp = tmpfile();
  REQUIRE(fp);
  REQUIRE(vfs(0, fp, "%f", 3.5) == 8);
  REQUIRE(slurp(fp) == "3.500000");
  fclose(fp);
}

TEST_CASE("shell va_list survives the two-pass measure/render")
{
  char buf[64];

  // the legacy bounded path measures first and renders again with the
  // same va_list; on the gcc Windows ABIs va_list is a plain pointer
  // and reusable — verify end to end through the shell
  va_list_probe(buf, sizeof buf, "%d %s %ls", 42, "x", L"你");
  REQUIRE(strcmp(buf, "42 x \xe4\xbd\xa0") == 0);
}
