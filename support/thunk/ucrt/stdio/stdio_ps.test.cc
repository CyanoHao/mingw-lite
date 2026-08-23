#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

// IAT-level tests for the _p/_s __stdio_common_* variants (plan §M4):
// positional arguments reach the musl engine natively; the memory
// _s/_p shapes are the wine-anchored truncation layouts (cap = count+1
// for _s, the shifted boundary with conditional terminator for _p);
// the stream variants are pure forwarders onto the M2/M3 shells.
// Direct symbol references are the documented mechanism
// (stdio_shell.test.cc precedent).

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
  int __cdecl __stdio_common_vsprintf_p(
      unsigned __int64 options,
      char *str,
      size_t count,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vsprintf_s(
      unsigned __int64 options,
      char *str,
      size_t count,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfprintf_p(
      unsigned __int64 options,
      FILE *stream,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfprintf_s(
      unsigned __int64 options,
      FILE *stream,
      const char *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vswprintf(
      unsigned __int64 options,
      wchar_t *str,
      size_t len,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vswprintf_p(
      unsigned __int64 options,
      wchar_t *str,
      size_t count,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vswprintf_s(
      unsigned __int64 options,
      wchar_t *str,
      size_t count,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfwprintf_p(
      unsigned __int64 options,
      FILE *stream,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
  int __cdecl __stdio_common_vfwprintf_s(
      unsigned __int64 options,
      FILE *stream,
      const wchar_t *format,
      const void *locale,
      va_list arglist);
}
#endif

namespace
{
  constexpr unsigned char kSentinel = 0xEE;
  constexpr wchar_t kWSentinel = (wchar_t)0xAAAA;

  // 你 / 好 as UTF-8 (kept split-escaped: greedy hex literals)
  const char *kNi = "\xe4\xbd\xa0";
  const char *kHao = "\xe5\xa5\xbd";
  const wchar_t *kWNi = L"\x4f60";
  // U+1F600 as a surrogate pair
  const wchar_t *kEmoji = L"\xd83d\xde00";

  int ps_p(unsigned __int64 o, char *s, size_t n, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsprintf_p(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int ps_s(unsigned __int64 o, char *s, size_t n, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsprintf_s(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pw_p(unsigned __int64 o, wchar_t *s, size_t n, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vswprintf_p(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pw_s(unsigned __int64 o, wchar_t *s, size_t n, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vswprintf_s(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pf_p(unsigned __int64 o, FILE *fp, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfprintf_p(o, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pf_s(unsigned __int64 o, FILE *fp, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfprintf_s(o, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pwf_p(unsigned __int64 o, FILE *fp, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfwprintf_p(o, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pwf_s(unsigned __int64 o, FILE *fp, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vfwprintf_s(o, fp, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int ps_plain(unsigned __int64 o, char *s, size_t n, const char *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vsprintf(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
  int pw_plain(unsigned __int64 o, wchar_t *s, size_t n, const wchar_t *f, ...)
  {
    va_list ap;
    va_start(ap, f);
    int r = __stdio_common_vswprintf(o, s, n, f, nullptr, ap);
    va_end(ap);
    return r;
  }
} // namespace

TEST_CASE("stdio_ps positional narrow")
{
  char b[64];

  // reorder
  int r = ps_p(0, b, sizeof b, "%2$s-%1$s", "a", "b");
  REQUIRE(r == 3);
  REQUIRE(strcmp(b, "b-a") == 0);

  // positional star width (wine anchor: "     5", ret 6)
  r = ps_p(0, b, sizeof b, "%1$*2$d", 5, 6);
  REQUIRE(r == 6);
  REQUIRE(strcmp(b, "     5") == 0);

  // reorder with mixed lengths
  r = ps_p(0, b, sizeof b, "%2$d/%3$lld/%1$d", 1, 22, 333LL);
  REQUIRE(r == 8);
  REQUIRE(strcmp(b, "22/333/1") == 0);

  // positional with CJK format text (byte counting includes UTF-8);
  // all directives positional — mixing is UB in the engine
  r = ps_p(0, b, sizeof b, "%2$s %3$s%1$d%4$s", 7, "x", kNi, kHao);
  REQUIRE(r == 9); // "x "(2) + 你(3) + "7"(1) + 好(3)
  REQUIRE(strcmp(b, "x " "\xe4\xbd\xa0" "7" "\xe5\xa5\xbd") == 0);

  // _p accepts a non-positional format too (engine autodetects)
  r = ps_p(0, b, sizeof b, "%d-%s", 42, "x");
  REQUIRE(r == 4);
  REQUIRE(strcmp(b, "42-x") == 0);

  // positional precision from an argument
  r = ps_p(0, b, sizeof b, "%1$.*2$f", 3.14159, 2);
  REQUIRE(r == 4);
  REQUIRE(strcmp(b, "3.14") == 0);
}

TEST_CASE("stdio_ps positional wide")
{
  wchar_t b[64];

  // ISO mode (no 0x4): %ls takes wchar_t*, %s takes char* — the
  // argument binds by POSITION: %1$ls wants the wide 1st vararg,
  // %2$s the narrow 2nd
  int r = pw_p(0, b, 64, L"%2$s-%1$ls", L"w", "n");
  REQUIRE(r == 3);
  REQUIRE(wcscmp(b, L"n-w") == 0);

  // legacy 0x4: bare %s swaps to wide, %hs stays narrow — the wide
  // convention is per POSITION: %2$s reads position 2 as wchar_t*,
  // %1$hs reads position 1 as char*
  r = pw_p(0x4, b, 64, L"%2$s-%1$hs", "n", L"w");
  REQUIRE(r == 3);
  REQUIRE(wcscmp(b, L"w-n") == 0);

  // positional star width counts wide units
  r = pw_p(0, b, 64, L"%1$*2$d", 5, 6);
  REQUIRE(r == 6);
  REQUIRE(wcscmp(b, L"     5") == 0);

  // positional %1$ls with CJK (1 unit) — wchar counting
  r = pw_p(0, b, 64, L"%1$ls-%2$d", kWNi, 42);
  REQUIRE(r == 4);
  REQUIRE(wcscmp(b, L"\x4f60-42") == 0);

  // emoji = surrogate pair = 2 units
  wmemset(b, kWSentinel, 64);
  r = pw_p(0, b, 64, L"%1$ls", kEmoji);
  REQUIRE(r == 2);
  REQUIRE(b[0] == (wchar_t)0xd83d);
  REQUIRE(b[1] == (wchar_t)0xde00);
  REQUIRE(b[2] == 0);
}

TEST_CASE("stdio_ps plain-entry positional (musl divergence anchor)")
{
  // The plain entries route through the musl engine, which interprets
  // %N$ — native ucrtbase prints the directive literally ("$s$s",
  // wine m4probe).  Deliberate musl-pure divergence (plan M4.6-1).
  char b[64];
  int r = ps_plain(0x1, b, sizeof b, "%2$s%1$s", "a", "b");
  REQUIRE(r == 2);
  REQUIRE(strcmp(b, "ba") == 0);

  wchar_t wb[64];
  r = pw_plain(0x1, wb, 64, L"%2$ls%1$ls", L"a", L"b");
  REQUIRE(r == 2);
  REQUIRE(wcscmp(wb, L"ba") == 0);
}

TEST_CASE("stdio_ps vsprintf_s matrix (wine-anchored)")
{
  unsigned char b[32];

  // fits: data + terminator at [need]
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_s(0, (char *)b, 8, "%s", "abc") == 3);
  REQUIRE(memcmp(b, "abc", 3) == 0);
  REQUIRE(b[3] == 0);
  REQUIRE(b[4] == kSentinel);

  // boundary need == count (wine s_eq3): ret 3, NUL lands AT [count]
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_s(0, (char *)b, 3, "%s", "abc") == 3);
  REQUIRE(memcmp(b, "abc", 3) == 0);
  REQUIRE(b[3] == 0);
  REQUIRE(b[4] == kSentinel);

  // truncation (wine s_trunc8): count data + NUL@[count], ret -1
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_s(0, (char *)b, 8, "% 15d", 1) == -1);
  for (int i = 0; i < 8; i++)
    REQUIRE(b[i] == ' ');
  REQUIRE(b[8] == 0);
  REQUIRE(b[9] == kSentinel);

  // count == 0 (wine s_cnt0): only the terminator at [0], ret -1
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_s(0, (char *)b, 0, "%d", 1) == -1);
  REQUIRE(b[0] == 0);
  REQUIRE(b[1] == kSentinel);

  // CJK exact fit: 你 is 3 bytes, need == count
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_s(0, (char *)b, 3, "%s", kNi) == 3);
  REQUIRE(memcmp(b, "\xe4\xbd\xa0", 3) == 0);
  REQUIRE(b[3] == 0);

  // rejection shapes: null buffer / null format (our EINVAL stance;
  // wine's null-buffer quirk returns the length — plan M4.6-2)
  errno = 0;
  REQUIRE(ps_s(0, nullptr, 8, "%d", 1) == -1);
  REQUIRE(errno == EINVAL);
  errno = 0;
  char buf[8];
  REQUIRE(ps_s(0, buf, 8, nullptr) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("stdio_ps vswprintf_s matrix (wine-anchored)")
{
  wchar_t b[32];

  // fits
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 8, L"%ls", L"abc") == 3);
  REQUIRE(wcscmp(b, L"abc") == 0);
  REQUIRE(b[4] == kWSentinel);

  // boundary need == count: terminator AT [count]
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 3, L"%ls", L"abc") == 3);
  REQUIRE(b[2] == L'c');
  REQUIRE(b[3] == 0);
  REQUIRE(b[4] == kWSentinel);

  // truncation (wine ws_trunc8): count units + NUL@[count], ret -1
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 8, L"% 15d", 1) == -1);
  for (int i = 0; i < 8; i++)
    REQUIRE(b[i] == L' ');
  REQUIRE(b[8] == 0);
  REQUIRE(b[9] == kWSentinel);

  // count == 0: only the terminator at [0]
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 0, L"%d", 1) == -1);
  REQUIRE(b[0] == 0);
  REQUIRE(b[1] == kWSentinel);

  // emoji pair fits exactly: 2 units + NUL@[2] (need == count)
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 2, L"%ls", kEmoji) == 2);
  REQUIRE(b[0] == (wchar_t)0xd83d);
  REQUIRE(b[1] == (wchar_t)0xde00);
  REQUIRE(b[2] == 0);
  REQUIRE(b[3] == kWSentinel);

  // ISO %s takes a narrow argument and widens it
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_s(0, b, 8, L"%s", "abc") == 3);
  REQUIRE(wcscmp(b, L"abc") == 0);
}

TEST_CASE("stdio_ps vsprintf_p matrix (wine-anchored)")
{
  unsigned char b[32];

  // fits: terminator at [need]
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_p(0, (char *)b, 8, "%s", "abc") == 3);
  REQUIRE(memcmp(b, "abc", 3) == 0);
  REQUIRE(b[3] == 0);
  REQUIRE(b[4] == kSentinel);

  // boundary need == count (wine p_eq3): data only, NO terminator
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_p(0, (char *)b, 3, "%s", "abc") == 3);
  REQUIRE(memcmp(b, "abc", 3) == 0);
  REQUIRE(b[3] == kSentinel);

  // truncation (wine p_trunc8): count data bytes, no terminator
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_p(0, (char *)b, 8, "% 15d", 1) == -1);
  for (int i = 0; i < 8; i++)
    REQUIRE(b[i] == ' ');
  REQUIRE(b[8] == kSentinel);

  // count == 0: zero bytes written
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_p(0, (char *)b, 0, "%d", 1) == -1);
  REQUIRE(b[0] == kSentinel);

  // CJK byte-granular truncation: 你 (3 bytes) into 2 -> 2-byte prefix
  memset(b, kSentinel, sizeof b);
  REQUIRE(ps_p(0, (char *)b, 2, "%s", kNi) == -1);
  REQUIRE(b[0] == 0xe4);
  REQUIRE(b[1] == 0xbd);
  REQUIRE(b[2] == kSentinel);
}

TEST_CASE("stdio_ps vswprintf_p matrix (wine-anchored)")
{
  wchar_t b[32];

  // fits: terminator at [need_w]
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_p(0, b, 8, L"%ls", L"abc") == 3);
  REQUIRE(wcscmp(b, L"abc") == 0);
  REQUIRE(b[4] == kWSentinel);

  // boundary: exactly count units, NO terminator
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_p(0, b, 3, L"%ls", L"abc") == 3);
  REQUIRE(b[2] == L'c');
  REQUIRE(b[3] == kWSentinel);

  // truncation (wine wp_trunc8): count units, no terminator, ret -1
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_p(0, b, 8, L"% 15d", 1) == -1);
  for (int i = 0; i < 8; i++)
    REQUIRE(b[i] == L' ');
  REQUIRE(b[8] == kWSentinel);

  // count == 0: zero units written
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_p(0, b, 0, L"%d", 1) == -1);
  REQUIRE(b[0] == kWSentinel);

  // emoji pair fits exactly: 2 units, no terminator (need == count)
  wmemset(b, kWSentinel, 32);
  REQUIRE(pw_p(0, b, 2, L"%ls", kEmoji) == 2);
  REQUIRE(b[0] == (wchar_t)0xd83d);
  REQUIRE(b[1] == (wchar_t)0xde00);
  REQUIRE(b[2] == kWSentinel);
}

TEST_CASE("stdio_ps stream forwarders")
{
  char b[64];
  size_t n;

  FILE *fp = tmpfile();
  REQUIRE(fp != nullptr);

  // narrow _p positional on a stream (wine anchor: "ba.", ret 3)
  REQUIRE(pf_p(0, fp, "%2$s%1$s.", "a", "b") == 3);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "ba.") == 0);

  // a native byte write interleaves cleanly with the bridge
  REQUIRE(fputs("Z", fp) >= 0);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "ba.Z") == 0);

  // narrow _s overwrites from the start (3 bytes of a 4-byte file)
  rewind(fp);
  REQUIRE(pf_s(0, fp, "%d-%d", 1, 2) == 3);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "1-2Z") == 0);

  // wide _p: the bridge stores UTF-8 bytes (2 bytes here)
  rewind(fp);
  REQUIRE(pwf_p(0, fp, L"%2$ls%1$ls", L"a", L"b") == 2);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "ba2Z") == 0);

  // wide _s with CJK, ISO %s takes a narrow argument (wine anchor: ret 2)
  rewind(fp);
  REQUIRE(pwf_s(0, fp, L"%s", "hi") == 2);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "hi2Z") == 0);

  // wide _p with CJK via %ls: UTF-8 bytes on the stream — the stream
  // return counts BYTES (known limit, plan M3.6-1): 你 = 3
  rewind(fp);
  REQUIRE(pwf_p(0, fp, L"%1$ls", kWNi) == 3);
  fflush(fp);
  rewind(fp);
  n = fread(b, 1, 10, fp);
  b[n] = 0;
  REQUIRE(strcmp(b, "\xe4\xbd\xa0" "Z") == 0); // 3 bytes over "hi2", Z survives

  fclose(fp);
}

TEST_CASE("stdio_ps %n is always enabled (musl semantics)")
{
  char b[64];
  int n = -1;

  // positional %n counts unconditionally on the _p entry
  n = 0;
  REQUIRE(ps_p(0, b, sizeof b, "%1$d%2$n", 5, &n) == 1);
  REQUIRE(n == 1);

  // the count-output switch is pinned on: set cannot close it
  REQUIRE(_set_printf_count_output(0) == 1);
  REQUIRE(_get_printf_count_output() == 1);

  // %n stays enabled on the _s entry
  n = 0;
  REQUIRE(ps_s(0, b, sizeof b, "%d%n", 5, &n) == 1);
  REQUIRE(n == 1);

  // and on the wide _p entry (positional %n through the translator)
  wchar_t wb[64];
  n = 0;
  REQUIRE(pw_p(0, wb, 64, L"%1$d%2$n", 5, &n) == 1);
  REQUIRE(n == 1);

  // the 0x10 three-digit exponent is orthogonal on the _s entry
  REQUIRE(ps_s(0, b, sizeof b, "%e", 1.5) == 12);
  REQUIRE(strcmp(b, "1.500000e+00") == 0);
  REQUIRE(ps_s(0x10, b, sizeof b, "%e", 1.5) == 13);
  REQUIRE(strcmp(b, "1.500000e+000") == 0);
}
