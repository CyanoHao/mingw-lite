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

#include "wfmt_translate.h"

using mingw_thunk::i::shell::wfmt_translate;

// IAT-level tests for the wide __stdio_common_v*w* family (plan §M3):
// every assertion goes through the overlay symbols, so the real path
// (pre-translation, UTF-8 engine, transcode back, stream/memory/
// console routing, options bits, %n gate, SECURECRT passthrough) is
// exercised.  Direct symbol references are the documented mechanism
// (stdio_shell.test.cc precedent).

#ifndef _UCRT
extern "C"
{
  // _locale_t is opaque to the shells; a void pointer matches the ABI
  int __cdecl __stdio_common_vswprintf(
      unsigned __int64 options,
      wchar_t *str,
      size_t len,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vsnwprintf_s(
      unsigned __int64 options,
      wchar_t *str,
      size_t size,
      size_t maxcount,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfwprintf(
      unsigned __int64 options,
      FILE *stream,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfwscanf(
      unsigned __int64 options,
      FILE *stream,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vswscanf(
      unsigned __int64 options,
      const wchar_t *input,
      size_t length,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
}
#endif

namespace
{
  constexpr wchar_t kSentinel = (wchar_t)0xAAAA;

  int wmem(unsigned __int64 options, wchar_t *str, size_t len, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vswprintf(options, str, len, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int wmems(
      unsigned __int64 options,
      wchar_t *str,
      size_t size,
      size_t maxcount,
      const wchar_t *f,
      ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsnwprintf_s(options, str, size, maxcount, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int wstream(unsigned __int64 options, FILE *fp, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfwprintf(options, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int wscan_stream(unsigned __int64 options, FILE *fp, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfwscanf(options, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }

  int wscan(unsigned __int64 options, const wchar_t *input, size_t length, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vswscanf(options, input, length, f, nullptr, ap);
    va_end(ap);
    return r;
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

  // one va_list driven through two wide shell calls (measure + render)
  void wva_probe(wchar_t *out, size_t n, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int need = __stdio_common_vswprintf(0x2, nullptr, 0, f, nullptr, ap);
    int r = __stdio_common_vswprintf(0x2, out, n, f, nullptr, ap);
    va_end(ap);
    REQUIRE(need >= 0);
    REQUIRE(r == need);
  }
} // namespace

TEST_CASE("wfmt translator truth table")
{
  char buf[128];

  auto tr = [&](const wchar_t *f, bool scanf_mode, bool legacy_wide) {
    wfmt_translate(f, scanf_mode, legacy_wide, buf, sizeof buf);
    return std::string(buf);
  };

  // printf ISO (no 0x4): zero rewrite — engine musl semantics
  CHECK(tr(L"[%s][%c][%ls][%hs][%S][%C]", false, false) ==
        "[%s][%c][%ls][%hs][%S][%C]");

  // printf legacy (0x4): the wide conventions realign s/c arguments
  CHECK(tr(L"[%s][%c][%hs][%hc][%ls][%lc][%ws][%S][%C]", false, true) ==
        "[%ls][%lc][%s][%c][%ls][%lc][%ls][%s][%c]");

  // scanf: rewrites are unconditional (both conventions write wide)
  CHECK(tr(L"[%s][%c][%hs][%hc][%ls][%lc][%ws]", true, false) ==
        "[%ls][%lc][%s][%c][%ls][%lc][%ls]");
  CHECK(tr(L"[%s]", true, true) == "[%ls]"); // legacy bit is irrelevant

  // prefix runs (positional, flags, width, precision, '*') are kept
  CHECK(tr(L"%1$-05.3ls", false, true) == "%1$-05.3ls");
  CHECK(tr(L"%*s", true, false) == "%*ls");
  CHECK(tr(L"%.*ls", false, true) == "%.*ls");
  CHECK(tr(L"%2$*3$d", false, true) == "%2$*3$d"); // untouched family

  // scanset rewrites keep the class verbatim (scanf only)
  CHECK(tr(L"%[0-9]", true, false) == "%l[0-9]");
  CHECK(tr(L"%h[a-z]", true, false) == "%[a-z]");
  CHECK(tr(L"%[]]", true, false) == "%l[]]");
  CHECK(tr(L"%[^]]", true, false) == "%l[^]]");
  CHECK(tr(L"%[x]", false, true) == "%[x]"); // '[' is no printf conv

  // %% rides verbatim; %n is copied verbatim in every shape
  CHECK(tr(L"100%% done", false, true) == "100%% done");
  CHECK(tr(L"%d%n", false, false) == "%d%n");
  CHECK(tr(L"%d%%n", false, false) == "%d%%n");
  CHECK(tr(L"%hn", true, false) == "%hn");

  // text transcoding: CJK and a surrogate pair
  CHECK(tr(L"\x4f60\x597d-%s", false, true) == "\xe4\xbd\xa0\xe5\xa5\xbd-%ls");
  CHECK(tr(L"\xd83d\xde00", false, true) == "\xf0\x9f\x98\x80");

  // trailing/malformed directives copy verbatim
  CHECK(tr(L"100%", false, true) == "100%");
  CHECK(tr(L"%5", false, true) == "%5");
  CHECK(tr(L"%hh", true, false) == "%hh");

  // clip-and-count: a short buffer reports the full requirement
  char small[4];
  CHECK(wfmt_translate(L"%s abc", false, true, small, sizeof small) == 8);
  CHECK(small[0] == '%');
  CHECK(small[3] == '\0');
}

TEST_CASE("wide ISO mode anchors the wine ucrtbase behavior")
{
  wchar_t b[32];

  // without 0x4, %s consumes a narrow UTF-8 string
  REQUIRE(wmem(0, b, sizeof b / sizeof *b, L"[%s]", "AB") == 4);
  REQUIRE(wcscmp(b, L"[AB]") == 0);
  REQUIRE(wmem(0, b, sizeof b / sizeof *b, L"[%s]", "\xe4\xbd\xa0") == 3);
  REQUIRE(wcscmp(b, L"[\x4f60]") == 0);

  // %ls still consumes wchar_t*; %c consumes an int (musl semantics)
  REQUIRE(wmem(0, b, sizeof b / sizeof *b, L"[%ls]", L"\x4f60\x597d") == 4);
  REQUIRE(wcscmp(b, L"[\x4f60\x597d]") == 0);
  REQUIRE(wmem(0, b, sizeof b / sizeof *b, L"[%c]", (int)'A') == 3);
  REQUIRE(wcscmp(b, L"[A]") == 0);
}

TEST_CASE("wide legacy mode (0x4) swaps the s/c conventions")
{
  wchar_t b[32];

  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%s]", L"\x4f60\x597d") == 4);
  REQUIRE(wcscmp(b, L"[\x4f60\x597d]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%hs]", "\xe4\xbd\xa0") == 3);
  REQUIRE(wcscmp(b, L"[\x4f60]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%c]", (wint_t)L'A') == 3);
  REQUIRE(wcscmp(b, L"[A]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%hc]", (int)'B') == 3);
  REQUIRE(wcscmp(b, L"[B]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%ls]", L"\x4f60") == 3);
  REQUIRE(wcscmp(b, L"[\x4f60]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%lc]", (wint_t)L'C') == 3);
  REQUIRE(wcscmp(b, L"[C]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%ws]", L"\x4f60") == 3);
  REQUIRE(wcscmp(b, L"[\x4f60]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%S]", "xy") == 4);
  REQUIRE(wcscmp(b, L"[xy]") == 0);
  REQUIRE(wmem(4, b, sizeof b / sizeof *b, L"[%C]", (int)'z') == 3);
  REQUIRE(wcscmp(b, L"[z]") == 0);
}

TEST_CASE("wide memory: vswprintf matrix (0x2 / 0x1 bounded / unbounded)")
{
  wchar_t b[32];

  // C11: fits — the return value counts wchar units
  REQUIRE(wmem(0x2, b, sizeof b / sizeof *b, L"[%ls]", L"hello") == 7);
  REQUIRE(wcscmp(b, L"[hello]") == 0);

  // CJK counts units, not bytes (2 here, not 6)
  REQUIRE(wmem(0x2, b, sizeof b / sizeof *b, L"%ls", L"\x4f60\x597d") == 2);
  // an emoji surrogate pair is exactly 2 units
  REQUIRE(wmem(0x2, b, sizeof b / sizeof *b, L"%ls", L"\xd83d\xde00") == 2);
  REQUIRE(wcscmp(b, L"\xd83d\xde00") == 0);

  // C11 truncation: required length survives, terminator kept, the
  // unit past the cap stays untouched
  wchar_t t[8];
  wmemset(t, kSentinel, 8);
  REQUIRE(wmem(0x2, t, 4, L"[%ls]", L"hello") == 7);
  REQUIRE(wmemcmp(t, L"[he", 3) == 0);
  REQUIRE(t[3] == L'\0');
  REQUIRE(t[4] == kSentinel);

  // measurement (str == NULL, len == 0) counts wide units
  REQUIRE(wmem(0x2, nullptr, 0, L"[%ls] %d", L"hi", 9) == 6);

  // legacy bounded: fits when need < len
  REQUIRE(wmem(1, t, 6, L"[%ls]", L"hi") == 4);
  REQUIRE(wcscmp(t, L"[hi]") == 0);

  // legacy bounded: need == len is already truncation — exactly len
  // units, NO terminator, -1
  wmemset(t, kSentinel, 8);
  REQUIRE(wmem(1, t, 4, L"[%ls]", L"hi") == -1);
  REQUIRE(wmemcmp(t, L"[hi]", 4) == 0);
  REQUIRE(t[4] == kSentinel);

  // legacy bounded: need == len-1 still fits
  REQUIRE(wmem(1, t, 5, L"[%ls]", L"hi") == 4);
  REQUIRE(wcscmp(t, L"[hi]") == 0);

  // heap scratch path (the render exceeds the 512-byte stack buffer)
  wchar_t big[700];
  wmemset(big, kSentinel, 700);
  REQUIRE(wmem(0x2, big, 700, L"%600d", 5) == 600);
  REQUIRE(big[0] == L' ');
  REQUIRE(big[599] == L'5');
  REQUIRE(big[600] == L'\0');
  REQUIRE(big[601] == kSentinel);

  // legacy unbounded
  REQUIRE(wmem(1, b, (size_t)-1, L"%ls-%d", L"x", 9) == 3);
  REQUIRE(wcscmp(b, L"x-9") == 0);

  // rejections
  errno = 0;
  REQUIRE(wmem(0x2, nullptr, 8, L"%d", 1) == -1);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wmem(0x2, b, 8, nullptr) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("wide memory: vsnwprintf_s truncation matrix")
{
  wchar_t b[16];

  // roomy maxcount: cap = maxcount + 1
  wmemset(b, kSentinel, 16);
  REQUIRE(wmems(0, b, 16, 10, L"[%ls]", L"hi") == 4);
  REQUIRE(wcscmp(b, L"[hi]") == 0);

  // _TRUNCATE: cap = size
  REQUIRE(wmems(0, b, 8, (size_t)-1, L"[%ls]", L"hello") == 7);
  REQUIRE(wcscmp(b, L"[hello]") == 0);

  // tight maxcount: cap = 4, need = 4 -> truncation with terminator
  wmemset(b, kSentinel, 16);
  REQUIRE(wmems(0, b, 16, 3, L"[%ls]", L"hi") == -1);
  REQUIRE(wmemcmp(b, L"[hi", 3) == 0);
  REQUIRE(b[3] == L'\0');
  REQUIRE(b[4] == kSentinel);

  // rejections
  errno = 0;
  REQUIRE(wmems(0, b, 0, 4, L"%d", 1) == -1);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wmems(0, nullptr, 16, 4, L"%d", 1) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("wide stream: byte bridge through a native FILE (tmpfile)")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);

  // ISO %s writes narrow bytes, legacy %ls writes wide — both land
  // UTF-8 exactly like the narrow family (plan M3.1-5); the stream
  // return counts bytes (known limit, plan M3.6-1)
  REQUIRE(wstream(0, fp, L"%s=%d", "k", 9) == 3);
  fputc('!', fp);
  REQUIRE(wstream(4, fp, L"-%ls", L"\x4f60\x597d") == 7);
  fputs("end", fp);
  fflush(fp);
  REQUIRE(slurp(fp) == "k=9!-\xe4\xbd\xa0\xe5\xa5\xbd"
                       "end");

  // read side: the bridge pulls bytes and the engine scans UTF-8
  FILE *fr = tmpfile();
  REQUIRE(fr);
  fputs("10 hello 2.5", fr);
  rewind(fr);
  int a;
  wchar_t ws[16];
  double d;
  REQUIRE(wscan_stream(0, fr, L"%d %ls %lf", &a, ws, &d) == 3);
  REQUIRE(a == 10);
  REQUIRE(wcscmp(ws, L"hello") == 0);
  REQUIRE(d == 2.5);
  fclose(fr);

  fclose(fp);
}

TEST_CASE("wide %n is always enabled (musl semantics)")
{
  wchar_t b[32];
  int n = 0;

  REQUIRE(wmem(0x2, b, 32, L"ab%n", &n) == 2);
  // ASCII content: bytes == wide units (count-unit note, M3.6-1)
  REQUIRE(n == 2);

  // the count-output switch is pinned on: set cannot close it
  REQUIRE(_set_printf_count_output(0) == 1);
  REQUIRE(_get_printf_count_output() == 1);
  n = 0;
  REQUIRE(wmem(0x2, b, 32, L"ab%n", &n) == 2);
  REQUIRE(n == 2);
}

TEST_CASE("wide SECURECRT (options 0x1) stays native")
{
  // the _s wrappers interleave a size_t after each buffer pointer;
  // the whole call is routed to the native implementation
  wchar_t sb[8];
  REQUIRE(wscan(1, L"abc", (size_t)-1, L"%s", sb, (size_t)8) == 1);
  REQUIRE(wcscmp(sb, L"abc") == 0);

  int i;
  REQUIRE(wscan(1, L"17", (size_t)-1, L"%d", &i) == 1);
  REQUIRE(i == 17);
}

TEST_CASE("wide shell output is invariant under native locale state")
{
  wchar_t b[32];

  setlocale(LC_ALL, "");
  setlocale(LC_NUMERIC, "C");

  REQUIRE(wmem(0x2, b, 32, L"%f", 3.5) == 8);
  REQUIRE(wcscmp(b, L"3.500000") == 0);
  REQUIRE(wmem(0x2, b, 32, L"%ls", L"\x4f60\x597d") == 2);
  REQUIRE(wcscmp(b, L"\x4f60\x597d") == 0);

  FILE *fp = tmpfile();
  REQUIRE(fp);
  REQUIRE(wstream(0, fp, L"%f", 3.5) == 8);
  REQUIRE(slurp(fp) == "3.500000");
  fclose(fp);
}

TEST_CASE("wide va_list survives the two-pass measure/render")
{
  wchar_t b[32];

  wva_probe(b, 32, L"%d %ls %s", 42, L"\x4f60", "x");
  REQUIRE(wcscmp(b, L"42 \x4f60 x") == 0);
}

TEST_CASE("wide scanf: conversion matrix and input transcoding")
{
  wchar_t wb[8];
  char nb[8];
  int i;

  // bare %s writes wchar_t (both conventions, plan M3.1-3)
  REQUIRE(wscan(0, L"ab 7", (size_t)-1, L"%s %d", wb, &i) == 2);
  REQUIRE(wcscmp(wb, L"ab") == 0);
  REQUIRE(i == 7);

  // CJK round trip through the UTF-8 the engine scans
  REQUIRE(wscan(0, L"\x4f60\x597d", (size_t)-1, L"%s", wb) == 1);
  REQUIRE(wcscmp(wb, L"\x4f60\x597d") == 0);

  // %hs stays narrow
  REQUIRE(wscan(0, L"ab", (size_t)-1, L"%hs", nb) == 1);
  REQUIRE(strcmp(nb, "ab") == 0);

  // %c reads one wide char; %hc reads one narrow char
  wchar_t wc = 0;
  REQUIRE(wscan(0, L"AB", (size_t)-1, L"%c", &wc) == 1);
  REQUIRE(wc == L'A');
  char nc = 0;
  REQUIRE(wscan(0, L"Z", (size_t)-1, L"%hc", &nc) == 1);
  REQUIRE(nc == 'Z');

  // scansets write wide; the class body rides verbatim
  REQUIRE(wscan(0, L"12ab", (size_t)-1, L"%[0-9]", wb) == 1);
  REQUIRE(wcscmp(wb, L"12") == 0);
  // a ']' first after '[' is literal in the class
  REQUIRE(wscan(0, L"]x", (size_t)-1, L"%[]]", wb) == 1);
  REQUIRE(wcscmp(wb, L"]") == 0);

  // finite length is a read limit; a NUL inside the bound terminates
  REQUIRE(wscan(0, L"10 20", 2, L"%d", &i) == 1);
  REQUIRE(i == 10);
  REQUIRE(wscan(0, L"10", 10, L"%d", &i) == 1);
  REQUIRE(i == 10);

  // a lone surrogate in the input becomes U+FFFD for the engine
  REQUIRE(wscan(0, L"\xd800x", (size_t)-1, L"%s", wb) == 1);
  REQUIRE(wcscmp(wb, L"\xfffdx") == 0);
}
