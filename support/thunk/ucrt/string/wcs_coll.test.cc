#include <catch_amalgamated.hpp>

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

// The wide collation family.  Declared here because the tests drive the
// faces directly and the points the plan asks about (the surrogate pair
// order and the count unit) are not visible through a header macro.
extern "C"
{
  int __cdecl wcscoll(const wchar_t *s1, const wchar_t *s2);
  int __cdecl
  _wcscoll_l(const wchar_t *s1, const wchar_t *s2, _locale_t locale);
  int __cdecl _wcsicoll(const wchar_t *s1, const wchar_t *s2);
  int __cdecl
  _wcsicoll_l(const wchar_t *s1, const wchar_t *s2, _locale_t locale);
  int __cdecl _wcsncoll(const wchar_t *s1, const wchar_t *s2, size_t count);
  int __cdecl _wcsncoll_l(const wchar_t *s1,
                          const wchar_t *s2,
                          size_t count,
                          _locale_t locale);
  int __cdecl _wcsnicoll(const wchar_t *s1, const wchar_t *s2, size_t count);
  int __cdecl _wcsnicoll_l(const wchar_t *s1,
                           const wchar_t *s2,
                           size_t count,
                           _locale_t locale);
  size_t __cdecl wcsxfrm(wchar_t *dst, const wchar_t *src, size_t count);
  size_t __cdecl
  _wcsxfrm_l(wchar_t *dst, const wchar_t *src, size_t count, _locale_t locale);
} // extern "C"

namespace
{
  constexpr int kNlsCmpError = INT_MAX;

  // U+1F600 as a surrogate pair and U+E000 as one unit.  Code point
  // order says U+E000 < U+1F600; unit order says 0xD83D < 0xE000, the
  // other way round.  This pair is the whole reason the family exists.
  const wchar_t kEmoji[] = {0xD83D, 0xDE00, 0};
  const wchar_t kPrivate[] = {0xE000, 0};
} // namespace

TEST_CASE("wcscoll orders by code point, not by UTF-16 unit")
{
  // The basic order, in wine's normalised -1/0/1 shape.
  REQUIRE(wcscoll(L"a", L"b") == -1);
  REQUIRE(wcscoll(L"a", L"a") == 0);
  REQUIRE(wcscoll(L"b", L"a") == 1);
  REQUIRE(wcscoll(L"ab", L"abc") == -1);
  REQUIRE(wcscoll(L"", L"") == 0);

  // The surrogate pair sorts *after* U+E000, which `wcscmp` cannot do:
  // its units are 0xD83D/0xDE00, both below 0xE000.  wine answers the
  // unit order here; this layer answers the code point order (D15).
  REQUIRE(wcscoll(kPrivate, kEmoji) == -1);
  REQUIRE(wcscoll(kEmoji, kPrivate) == 1);

  // ... and the pair still sorts above every BMP character, including
  // the top of the BMP.
  REQUIRE(wcscoll(L"\uFFFF", kEmoji) == -1);
  REQUIRE(wcscoll(kEmoji, L"\uFFFF") == 1);
}

TEST_CASE("wcscoll validates both operands with _NLSCMPERROR")
{
  errno = 0;
  REQUIRE(wcscoll(nullptr, L"a") == kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wcscoll(L"a", nullptr) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wcscoll(nullptr, nullptr) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_wcsicoll folds by code point and answers the difference")
{
  REQUIRE(_wcsicoll(L"A", L"a") == 0);
  REQUIRE(_wcsicoll(L"Ab", L"aB") == 0);
  REQUIRE(_wcsicoll(L"A", L"b") == -1);
  REQUIRE(_wcsicoll(L"A", L"c") == -2); // folded 'a' - 'c'
  REQUIRE(_wcsicoll(L"Z", L"a") == 25); // folded 'z' - 'a'
}

TEST_CASE("_wcsicoll validates both operands")
{
  errno = 0;
  REQUIRE(_wcsicoll(nullptr, L"a") == kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_wcsicoll(L"a", nullptr) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_wcsncoll bounds by wide characters and answers the difference")
{
  REQUIRE(_wcsncoll(L"ab", L"ac", 1) == 0);
  REQUIRE(_wcsncoll(L"ab", L"ac", 2) == -1);
  REQUIRE(_wcsncoll(L"a", L"c", 1) == -2);
  REQUIRE(_wcsncoll(L"z", L"a", 1) == 25);
  REQUIRE(_wcsncoll(L"ab", L"ab", 5) == 0);

  // A zero count answers before anything is looked at, so null
  // operands are not a validation failure at zero.
  errno = ERANGE;
  REQUIRE(_wcsncoll(nullptr, nullptr, 0) == 0);
  REQUIRE(errno == ERANGE);
}

TEST_CASE("_wcsncoll validates a live count and a null operand")
{
  errno = 0;
  REQUIRE(_wcsncoll(nullptr, L"a", 1) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_wcsncoll(L"a", nullptr, 1) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_wcsncoll(L"a", L"b", (size_t)INT_MAX + 1) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_wcsnicoll is the counted fold compare")
{
  REQUIRE(_wcsnicoll(L"AB", L"ac", 1) == 0);
  REQUIRE(_wcsnicoll(L"AB", L"ac", 2) == -1);
  REQUIRE(_wcsnicoll(L"Z", L"a", 1) == 25);

  errno = ERANGE;
  REQUIRE(_wcsnicoll(nullptr, nullptr, 0) == 0);
  REQUIRE(errno == ERANGE);
  errno = 0;
  REQUIRE(_wcsnicoll(nullptr, L"a", 1) == kNlsCmpError);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("the counted bound never splits a surrogate pair")
{
  // Two units are available and the pair needs both, so it is compared
  // whole: the difference is the code point difference U+1F600 - U+E000
  // (this face answers the difference, like wine's `wcsncmp` path).
  REQUIRE(_wcsncoll(kEmoji, kPrivate, 2) == 0x1F600 - 0xE000);
  // One unit is available and the pair needs two, so the comparison
  // ends at the bound and the prefixes are equal.
  REQUIRE(_wcsncoll(kEmoji, kPrivate, 1) == 0);
}

TEST_CASE("wcsxfrm is the identity transform in units")
{
  wchar_t dst[16];

  memset(dst, 0, sizeof dst);
  REQUIRE(wcsxfrm(dst, L"abc", 16) == 3);
  REQUIRE(wcscmp(dst, L"abc") == 0);

  // A source shorter than the window is NUL-padded to count, and the
  // bytes past count are left alone -- the reference's `wcsncpy`, and
  // wine's observable answer.
  for (size_t k = 0; k < 16; ++k)
    dst[k] = 0x7F;
  REQUIRE(wcsxfrm(dst, L"abc", 6) == 3);
  REQUIRE(dst[0] == L'a');
  REQUIRE(dst[1] == L'b');
  REQUIRE(dst[2] == L'c');
  REQUIRE(dst[3] == 0);
  REQUIRE(dst[5] == 0);
  REQUIRE(dst[6] == 0x7F);

  // A source longer than the window is truncated to count units with no
  // terminator, and the full length comes back.
  for (size_t k = 0; k < 16; ++k)
    dst[k] = 0x7F;
  REQUIRE(wcsxfrm(dst, L"abcdef", 3) == 6);
  REQUIRE(dst[0] == L'a');
  REQUIRE(dst[2] == L'c');
  REQUIRE(dst[3] == 0x7F);

  // A surrogate pair is two units each way, and the length is in units.
  for (size_t k = 0; k < 16; ++k)
    dst[k] = 0x7F;
  REQUIRE(wcsxfrm(dst, kEmoji, 16) == 2);
  REQUIRE(dst[0] == 0xD83D);
  REQUIRE(dst[1] == 0xDE00);
  REQUIRE(dst[2] == 0);

  // A zero count writes nothing and still answers the length; a null
  // destination is allowed exactly when the count is zero.
  for (size_t k = 0; k < 16; ++k)
    dst[k] = 0x7F;
  REQUIRE(wcsxfrm(dst, L"abc", 0) == 3);
  REQUIRE(dst[0] == 0x7F);
  REQUIRE(wcsxfrm(nullptr, L"abc", 0) == 3);
}

TEST_CASE("wcsxfrm reports its validation failures with _NLSCMPERROR")
{
  wchar_t dst[16];

  errno = 0;
  REQUIRE(wcsxfrm(dst, nullptr, 16) == (size_t)kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wcsxfrm(nullptr, L"abc", 16) == (size_t)kNlsCmpError);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(wcsxfrm(dst, L"abc", (size_t)INT_MAX + 1) == (size_t)kNlsCmpError);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("every wide collation face ignores a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  wchar_t dst[16];

  REQUIRE(_wcscoll_l(L"a", L"b", anywhere) == -1);
  REQUIRE(_wcscoll_l(kEmoji, kPrivate, anywhere) == 1);
  REQUIRE(_wcsicoll_l(L"A", L"c", anywhere) == -2);
  REQUIRE(_wcsncoll_l(L"ab", L"ac", 2, anywhere) == -1);
  REQUIRE(_wcsncoll_l(nullptr, L"a", 0, anywhere) == 0);
  REQUIRE(_wcsnicoll_l(L"AB", L"ac", 2, anywhere) == -1);
  errno = 0;
  REQUIRE(_wcsicoll_l(nullptr, L"a", anywhere) == kNlsCmpError);
  REQUIRE(errno == EINVAL);

  memset(dst, 0, sizeof dst);
  REQUIRE(_wcsxfrm_l(dst, L"abc", 16, anywhere) == 3);
  REQUIRE(wcscmp(dst, L"abc") == 0);
}
