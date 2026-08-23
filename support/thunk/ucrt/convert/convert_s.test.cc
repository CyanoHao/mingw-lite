#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

// Overlay-bound secure conversions (wine-anchored M7 protocol).
extern "C" int __cdecl mbstowcs_s(size_t *conv, wchar_t *dst, size_t size,
                                  const char *src, size_t count);
extern "C" int __cdecl wcstombs_s(size_t *conv, char *dst, size_t size,
                                  const wchar_t *src, size_t count);
extern "C" int __cdecl wctomb_s(int *size_converted, char *s, size_t size,
                                wchar_t wc);
extern "C" int __cdecl mbsrtowcs_s(size_t *retval, wchar_t *dst, size_t size,
                                   const char **src, size_t n,
                                   mbstate_t *ps);
extern "C" int __cdecl wcsrtombs_s(size_t *retval, char *dst, size_t size,
                                   const wchar_t **src, size_t n,
                                   mbstate_t *ps);
extern "C" size_t __cdecl _wcstombs_l(char *dest, const wchar_t *source,
                                      size_t count, _locale_t locale);
extern "C" int __cdecl _wctomb_l(char *s, wchar_t wc, _locale_t locale);
extern "C" int __cdecl _wcstombs_s_l(size_t *conv, char *dst, size_t size,
                                     const wchar_t *src, size_t count,
                                     _locale_t locale);
extern "C" int __cdecl _wctomb_s_l(int *size_converted, char *s, size_t size,
                                   wchar_t wc, _locale_t locale);

#define STRUNCATE_RC 80

TEST_CASE("mbstowcs_s")
{
  size_t n;
  wchar_t wb[8];

  // counting mode: terminator included, _TRUNCATE accepted
  n = 0xdead;
  REQUIRE(mbstowcs_s(&n, nullptr, 0, "abc", 100) == 0);
  REQUIRE(n == 4);
  n = 0xdead;
  REQUIRE(mbstowcs_s(&n, nullptr, 0, "abc", _TRUNCATE) == 0);
  REQUIRE(n == 4);

  // exact fit
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 4, "abc", 100) == 0);
  REQUIRE(n == 4);
  REQUIRE(wcscmp(wb, L"abc") == 0);

  // too small without _TRUNCATE: ERANGE, reset, *conv = 0
  memset(wb, 0xAA, sizeof wb);
  n = 0xdead;
  REQUIRE(mbstowcs_s(&n, wb, 3, "abc", 100) == ERANGE);
  REQUIRE(n == 0);
  REQUIRE(wb[0] == 0);

  // _TRUNCATE: STRUNCATE with as much as fits (2 chars + NUL = 3)
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 3, "abc", _TRUNCATE) == STRUNCATE_RC);
  REQUIRE(n == 3);
  REQUIRE(wcscmp(wb, L"ab") == 0);

  // count limits the source
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, "abcd", 2) == 0);
  REQUIRE(n == 3);
  REQUIRE(wcscmp(wb, L"ab") == 0);

  // empty conversion (count == 0)
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, "abcd", 0) == 0);
  REQUIRE(n == 1);

  // wine anchors: (dst, 0) -> rc 0/*conv 0; dst null + size -> EINVAL;
  // null src -> EINVAL; null conv pointer tolerated
  n = 0xdead;
  REQUIRE(mbstowcs_s(&n, wb, 0, "abc", 10) == 0);
  REQUIRE(n == 0);
  REQUIRE(mbstowcs_s(&n, nullptr, 4, "abc", 10) == EINVAL);
  REQUIRE(mbstowcs_s(nullptr, wb, 8, "abc", 10) == 0);
  errno = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, nullptr, 10) == EINVAL);
  REQUIRE(errno == EINVAL);

  // CJK: 你 (3 UTF-8 bytes) -> single wchar
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, "\xe4\xbd\xa0", _TRUNCATE) == 0);
  REQUIRE(n == 2);
  REQUIRE(wb[0] == 0x4F60);

  // astral: 4-byte sequence splits into a surrogate pair
  n = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, "\xf0\x9f\x98\x80", _TRUNCATE) == 0);
  REQUIRE(n == 3);
  REQUIRE(wb[0] == 0xD83D);
  REQUIRE(wb[1] == 0xDE00);

  // invalid UTF-8: EILSEQ with reset
  memset(wb, 0xAA, sizeof wb);
  n = 0xdead;
  errno = 0;
  REQUIRE(mbstowcs_s(&n, wb, 8, "\xff\xfe", 2) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  REQUIRE(n == 0);
  REQUIRE(wb[0] == 0);
}

TEST_CASE("wcstombs_s")
{
  size_t n;
  char b[8];

  n = 0xdead;
  REQUIRE(wcstombs_s(&n, nullptr, 0, L"abc", 100) == 0);
  REQUIRE(n == 4);

  n = 0;
  REQUIRE(wcstombs_s(&n, b, 4, L"abc", 100) == 0);
  REQUIRE(n == 4);
  REQUIRE(strcmp(b, "abc") == 0);

  memset(b, 0xAA, sizeof b);
  n = 0xdead;
  REQUIRE(wcstombs_s(&n, b, 3, L"abc", 100) == ERANGE);
  REQUIRE(n == 0);
  REQUIRE(b[0] == 0);

  n = 0;
  REQUIRE(wcstombs_s(&n, b, 3, L"abc", _TRUNCATE) == STRUNCATE_RC);
  REQUIRE(n == 3);
  REQUIRE(strcmp(b, "ab") == 0);

  n = 0;
  REQUIRE(wcstombs_s(&n, b, 8, L"abcd", 2) == 0);
  REQUIRE(n == 3);
  REQUIRE(strcmp(b, "ab") == 0);

  // CJK wide -> 3 UTF-8 bytes
  n = 0;
  REQUIRE(wcstombs_s(&n, b, 8, L"\x4F60", _TRUNCATE) == 0);
  REQUIRE(n == 4);
  REQUIRE((unsigned char)b[0] == 0xe4);
  REQUIRE((unsigned char)b[1] == 0xbd);
  REQUIRE((unsigned char)b[2] == 0xa0);
  REQUIRE(b[3] == 0);

  // surrogate pair joins into one code point (4 bytes)
  n = 0;
  REQUIRE(wcstombs_s(&n, b, 8, L"\xd83d\xde00", _TRUNCATE) == 0);
  REQUIRE(n == 5);
  REQUIRE((unsigned char)b[0] == 0xf0);

  // unpaired surrogate: EILSEQ
  n = 0;
  errno = 0;
  REQUIRE(wcstombs_s(&n, b, 8, L"\xdc00", _TRUNCATE) == EILSEQ);
  REQUIRE(errno == EILSEQ);
}

TEST_CASE("wctomb_s")
{
  int len;
  char b[8];

  len = -1;
  REQUIRE(wctomb_s(&len, b, 8, L'a') == 0);
  REQUIRE(len == 1);
  REQUIRE(b[0] == 'a');

  // L'\0' stores exactly one NUL byte (wine anchor)
  len = -1;
  REQUIRE(wctomb_s(&len, b, 8, L'\0') == 0);
  REQUIRE(len == 1);
  REQUIRE(b[0] == 0);

  // CJK needs 3 bytes
  len = -1;
  REQUIRE(wctomb_s(&len, b, 8, 0x4F60) == 0);
  REQUIRE(len == 3);
  REQUIRE((unsigned char)b[0] == 0xe4);

  // convertible but too large: ERANGE
  len = -1;
  errno = 0;
  REQUIRE(wctomb_s(&len, b, 2, 0x4F60) == ERANGE);
  REQUIRE(len == -1);
  REQUIRE(errno == ERANGE);
  REQUIRE(b[0] == 0);

  // wine anchors: null buffer / zero size -> ERANGE, *len = -1
  len = -1;
  REQUIRE(wctomb_s(&len, nullptr, 0, L'a') == ERANGE);
  REQUIRE(len == -1);
  len = -1;
  REQUIRE(wctomb_s(&len, b, 0, L'a') == ERANGE);
  REQUIRE(len == -1);

  // lone surrogate: EILSEQ (M5 wctomb parity)
  len = -1;
  errno = 0;
  REQUIRE(wctomb_s(&len, b, 8, 0xD83D) == EILSEQ);
  REQUIRE(len == -1);
  REQUIRE(errno == EILSEQ);
}

TEST_CASE("mbsrtowcs_s")
{
  size_t n;
  wchar_t wb[8];
  const char *src;
  mbstate_t st;

  memset(&st, 0, sizeof st);
  src = "ab";
  n = 0xdead;
  REQUIRE(mbsrtowcs_s(&n, wb, 8, &src, 8, &st) == 0);
  REQUIRE(n == 3);           // terminator included
  REQUIRE(src == nullptr);   // complete-string shape (wine anchor)
  REQUIRE(wcscmp(wb, L"ab") == 0);

  // too-small dst: ERANGE, reset, *src past the last stored char
  memset(&st, 0, sizeof st);
  const char *base = "abcd";
  src = base;
  memset(wb, 0xAA, sizeof wb);
  n = 0xdead;
  REQUIRE(mbsrtowcs_s(&n, wb, 3, &src, 8, &st) == ERANGE);
  REQUIRE(n == 0);
  REQUIRE(wb[0] == 0);
  REQUIRE(src == base + 2);

  // counting mode: *src untouched
  memset(&st, 0, sizeof st);
  src = base;
  n = 0xdead;
  REQUIRE(mbsrtowcs_s(&n, nullptr, 0, &src, 8, &st) == 0);
  REQUIRE(n == 5);
  REQUIRE(src == base);
}

TEST_CASE("wcsrtombs_s")
{
  size_t n;
  char b[8];
  const wchar_t *src;
  mbstate_t st;

  memset(&st, 0, sizeof st);
  src = L"ab";
  n = 0xdead;
  REQUIRE(wcsrtombs_s(&n, b, 8, &src, 8, &st) == 0);
  REQUIRE(n == 3);
  REQUIRE(src == nullptr);
  REQUIRE(strcmp(b, "ab") == 0);

  // pair joins before the encode
  memset(&st, 0, sizeof st);
  src = L"\xd83d\xde00";
  n = 0;
  REQUIRE(wcsrtombs_s(&n, b, 8, &src, 2, &st) == 0);
  REQUIRE(n == 5);
  REQUIRE((unsigned char)b[0] == 0xf0);
  REQUIRE(src == nullptr);
}

TEST_CASE("convert_l_variants")
{
  char b[8];
  int len;

  // locale arguments are ignored entirely (null is safe)
  REQUIRE(_wctomb_l(b, L'a', nullptr) == 1);
  REQUIRE(b[0] == 'a');

  REQUIRE(_wcstombs_l(b, L"abc", 8, nullptr) == 3);
  REQUIRE(strcmp(b, "abc") == 0);

  len = -1;
  REQUIRE(_wctomb_s_l(&len, b, 8, 0x4F60, nullptr) == 0);
  REQUIRE(len == 3);

  size_t n = 0;
  REQUIRE(_wcstombs_s_l(&n, b, 8, L"\x4F60", 8, nullptr) == 0);
  REQUIRE(n == 4);
  REQUIRE((unsigned char)b[0] == 0xe4);
}

TEST_CASE("convert_s_native_locale_pollution")
{
  // Adversarial: native wide setlocale flips the NATIVE converter
  // state; the overlay conversions stay UTF-8 unconditionally
  _wsetlocale(LC_CTYPE, L"Chinese_China.936");

  size_t n = 0;
  wchar_t wb[4];
  REQUIRE(mbstowcs_s(&n, wb, 4, "\xe4\xbd\xa0", _TRUNCATE) == 0);
  REQUIRE(n == 2);
  REQUIRE(wb[0] == 0x4F60);

  char b[8];
  n = 0;
  REQUIRE(wcstombs_s(&n, b, 8, L"\x4F60", _TRUNCATE) == 0);
  REQUIRE(n == 4);
  REQUIRE((unsigned char)b[0] == 0xe4);

  _wsetlocale(LC_CTYPE, L"C");
}
