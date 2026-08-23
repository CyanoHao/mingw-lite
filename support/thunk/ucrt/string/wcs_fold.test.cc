#include <catch_amalgamated.hpp>

#include <errno.h>
#include <wchar.h>

typedef int errno_t;
#include <locale.h>

extern "C" int __cdecl _wcsicmp_l(const wchar_t *, const wchar_t *, _locale_t);
extern "C" int __cdecl _wcsnicmp_l(const wchar_t *, const wchar_t *, size_t,
                                   _locale_t);
extern "C" errno_t __cdecl _wcslwr_s(wchar_t *, size_t);
extern "C" errno_t __cdecl _wcsupr_s(wchar_t *, size_t);
extern "C" wchar_t *__cdecl _wcslwr_l(wchar_t *, _locale_t);
extern "C" wchar_t *__cdecl _wcsupr_l(wchar_t *, _locale_t);

TEST_CASE("_wcsicmp")
{
  REQUIRE(_wcsicmp(L"A", L"a") == 0);
  REQUIRE(_wcsicmp(L"b", L"d") == -2); // folded difference (wine shape)
  REQUIRE(_wcsicmp(L"HELLO", L"hello") == 0);
  REQUIRE(_wcsicmp(L"", L"") == 0);
  REQUIRE(_wcsicmp(L"abc", L"") == L'a');

  // C.UTF-8 divergence by design: the C-locale native folds ASCII
  // only (wine anchor: C9 vs E9 -> -32); ours folds via towlower -> 0
  REQUIRE(_wcsicmp(L"\x00C9", L"\x00E9") == 0);
  REQUIRE(_wcsicmp(L"\x00DC", L"\x00FC") == 0); // U+00DC / U+00FC
  REQUIRE(_wcsicmp(L"\x00C9", L"\x00EA") != 0);

  // CJK has no case
  REQUIRE(_wcsicmp(L"\x4F60\x597D", L"\x4F60\x597D") == 0);

  REQUIRE(_wcsicmp_l(L"Ab", L"aB", nullptr) == 0);
  REQUIRE(_wcsicmp_l(L"b", L"d", nullptr) == -2);
}

TEST_CASE("_wcsnicmp")
{
  REQUIRE(_wcsnicmp(L"Ab", L"aC", 2) == -1);
  REQUIRE(_wcsnicmp(L"AB", L"ab", 2) == 0);
  REQUIRE(_wcsnicmp(L"ABc", L"abd", 2) == 0);
  REQUIRE(_wcsnicmp(L"abc", L"abd", 3) == L'c' - L'd');
  REQUIRE(_wcsnicmp(L"A", L"a", 0) == 0);
  REQUIRE(_wcsnicmp(L"\x00C9x", L"\x00E9x", 2) == 0); // folds, then equal
  REQUIRE(_wcsnicmp_l(L"Ab", L"aC", 2, nullptr) == -1);
}

TEST_CASE("_wcslwr _wcsupr")
{
  wchar_t buf[8];

  wcscpy(buf, L"AbC");
  REQUIRE(_wcslwr(buf) == buf);
  REQUIRE(wcscmp(buf, L"abc") == 0);

  wcscpy(buf, L"aBc");
  REQUIRE(_wcsupr(buf) == buf);
  REQUIRE(wcscmp(buf, L"ABC") == 0);

  // wide fold covers Latin-1 (towlower engine); CJK unchanged
  wcscpy(buf, L"\x00C9\x4F60");
  REQUIRE(_wcslwr(buf) == buf);
  REQUIRE(buf[0] == 0x00E9);
  REQUIRE(buf[1] == 0x4F60);

  wcscpy(buf, L"\x00E9\x4F60");
  REQUIRE(_wcsupr(buf) == buf);
  REQUIRE(buf[0] == 0x00C9);
  REQUIRE(buf[1] == 0x4F60);

  errno = 0;
  REQUIRE(_wcslwr(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_wcsupr(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);

  wcscpy(buf, L"AbC");
  REQUIRE(_wcslwr_l(buf, nullptr) == buf);
  REQUIRE(wcscmp(buf, L"abc") == 0);
  wcscpy(buf, L"aBc");
  REQUIRE(_wcsupr_l(buf, nullptr) == buf);
  REQUIRE(wcscmp(buf, L"ABC") == 0);
}

TEST_CASE("_wcslwr_s _wcsupr_s")
{
  wchar_t buf[8];

  wcscpy(buf, L"AbC");
  errno = 0;
  REQUIRE(_wcslwr_s(buf, 8) == 0);
  REQUIRE(wcscmp(buf, L"abc") == 0);
  REQUIRE(errno == 0);

  wcscpy(buf, L"aBc");
  REQUIRE(_wcsupr_s(buf, 8) == 0);
  REQUIRE(wcscmp(buf, L"ABC") == 0);

  // too small -> EINVAL + reset (wine anchor: rc 22)
  wcscpy(buf, L"AbC");
  errno = 0;
  REQUIRE(_wcslwr_s(buf, 2) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[0] == 0);

  // size 0 -> EINVAL, untouched
  wcscpy(buf, L"AbC");
  REQUIRE(_wcslwr_s(buf, 0) == EINVAL);
  REQUIRE(buf[0] == L'A');

  // null -> EINVAL
  REQUIRE(_wcslwr_s(nullptr, 8) == EINVAL);
  REQUIRE(_wcsupr_s(nullptr, 8) == EINVAL);

  // Latin-1 fold through the _s path (concatenated literal: the hex
  // escape would otherwise swallow the trailing 'b')
  wcscpy(buf, L"\x00C9" L"b");
  REQUIRE(_wcslwr_s(buf, 8) == 0);
  REQUIRE(buf[0] == 0x00E9);
  REQUIRE(buf[1] == L'b');
}
