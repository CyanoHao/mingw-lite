#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

// i686 hides these from <stdlib.h>; x64 sees them and only warns, which
// is the M12 finding.  Declaring them keeps both arches quiet.
extern "C"
{
  int __cdecl mblen(const char *s, size_t n);
  int __cdecl mbtowc(wchar_t *pwc, const char *s, size_t n);
  size_t __cdecl mbstowcs(wchar_t *pwcs, const char *s, size_t n);
  errno_t __cdecl mbstowcs_s(
      size_t *conv, wchar_t *dst, size_t dstsz, const char *src, size_t count);

  int __cdecl _mblen_l(const char *s, size_t n, _locale_t locale);
  int __cdecl
  _mbtowc_l(wchar_t *pwc, const char *s, size_t n, _locale_t locale);
  size_t __cdecl
  _mbstowcs_l(wchar_t *pwcs, const char *s, size_t n, _locale_t locale);
  errno_t __cdecl _mbstowcs_s_l(size_t *conv,
                                wchar_t *dst,
                                size_t dstsz,
                                const char *src,
                                size_t count,
                                _locale_t locale);
} // extern "C"

namespace
{
  // "a" + U+3042 + U+1F600 + "z": one single byte, one three-byte
  // sequence and one four-byte sequence.
  const char kMixed[] = "a\xE3\x81\x82\xF0\x9F\x98\x80z";
  constexpr size_t kMixedLen = 9; // bytes, excluding the terminator

  // The whole-string conversions (mbsrtowcs / mbstowcs_s) expand U+1F600
  // into a surrogate pair, so "a" U+3042 D83D DE00 "z" is five wchar_t
  // plus a terminator.  mbtowc, which fills a single wchar_t, instead
  // degrades the non-BMP scalar to U+FFFD (M7's mbrtowc contract).
  constexpr size_t kMixedWc = 5; // wchar_t produced, terminator excluded
  constexpr size_t kSlots = kMixedWc + 1;
} // namespace

TEST_CASE("_mblen_l delegates to mblen")
{
  // mblen measures the *first* character, so kMixed itself is one byte
  // and kMixed + 1 is the three-byte sequence.
  REQUIRE(_mblen_l(kMixed, kMixedLen, nullptr) == mblen(kMixed, kMixedLen));
  REQUIRE(_mblen_l(kMixed, 1, nullptr) == 1);
  REQUIRE(_mblen_l(kMixed + 1, 3, nullptr) == 3);
  REQUIRE(_mblen_l(kMixed + 4, 4, nullptr) == 4);

  // The mbtowc reset protocol: a null pointer or a zero count clears
  // whatever state the previous call left behind and answers 0.
  REQUIRE(_mblen_l(kMixed, 0, nullptr) == 0);
  REQUIRE(_mblen_l(nullptr, 8, nullptr) == 0);

  // A non-null locale_t must not change the answer.
  _locale_t loc = _create_locale(LC_ALL, "C");
  REQUIRE(_mblen_l(kMixed + 1, 8, loc) == 3);
  _free_locale(loc);

  // A truncated sequence is *not* softened the way _mbclen softens it
  // (plan-3 D8d): mblen inherits mbtowc's contract, where an incomplete
  // character in the first n bytes is a failure.  Both -2 (incomplete)
  // and -1 (invalid) collapse to -1 through the mbtowc thunk.
  REQUIRE(_mblen_l("\xE3\x81", 2, nullptr) == -1);
  REQUIRE(_mblen_l("\xE3\x81", 2, nullptr) == mblen("\xE3\x81", 2));

  // With enough bytes the same character succeeds — but only from a
  // clean shift state.  mbtowc keeps one static mbstate_t, so a failed
  // call above left it mid-sequence; a zero-count call is the documented
  // reset.  This is the statefulness the D9 principle has to preserve.
  REQUIRE(_mblen_l(nullptr, 0, nullptr) == 0);
  REQUIRE(_mblen_l("\xE3\x81\x82", 3, nullptr) == 3);

  // ...and a fresh partial sequence poisons the next one, which is why
  // the reset exists.
  REQUIRE(_mblen_l("\xE3\x81", 2, nullptr) == -1);
  REQUIRE(_mblen_l("\xE3\x81\x82", 3, nullptr) == -1);
  REQUIRE(_mblen_l(nullptr, 0, nullptr) == 0);
  REQUIRE(_mblen_l("\xE3\x81\x82", 3, nullptr) == 3);

  // A byte that can never lead is a plain -1 and leaves no residue.
  REQUIRE(_mblen_l("\xFF", 1, nullptr) == -1);
  REQUIRE(_mblen_l("\xE3\x81\x82", 3, nullptr) == 3);
}

TEST_CASE("_mbtowc_l delegates to mbtowc")
{
  wchar_t w1 = 0xFFFF;
  wchar_t w2 = 0xFFFF;
  REQUIRE(_mbtowc_l(&w1, kMixed, kMixedLen, nullptr) ==
          mbtowc(&w2, kMixed, kMixedLen));
  REQUIRE(w1 == w2);
  REQUIRE(w1 == L'a');

  wchar_t w3 = 0;
  REQUIRE(_mbtowc_l(&w3, kMixed + 1, kMixedLen - 1, nullptr) == 3);
  REQUIRE(w3 == 0x3042);

  // A four-byte sequence is four bytes wide, but its scalar does not fit
  // a single wchar_t, so mbrtowc degrades it to U+FFFD rather than
  // emitting a half surrogate (M7's mbrtowc contract).  The whole-string
  // conversions below are the ones that pair the surrogates.
  wchar_t w4 = 0;
  REQUIRE(_mbtowc_l(&w4, kMixed + 4, kMixedLen - 4, nullptr) == 4);
  REQUIRE(w4 == 0xFFFD);

  // nullptr destination is a valid query: length only.
  REQUIRE(_mbtowc_l(nullptr, kMixed, kMixedLen, nullptr) == 1);
  REQUIRE(_mbtowc_l(nullptr, kMixed + 1, kMixedLen - 1, nullptr) == 3);

  // Empty string: the NUL is reported as a zero-width conversion.
  wchar_t w5 = 0xFFFF;
  REQUIRE(_mbtowc_l(&w5, "", 1, nullptr) == 0);
  REQUIRE(w5 == 0);

  _locale_t loc = _create_locale(LC_ALL, "C");
  wchar_t w6 = 0;
  REQUIRE(_mbtowc_l(&w6, kMixed + 1, 8, loc) == 3);
  REQUIRE(w6 == 0x3042);
  _free_locale(loc);
}

TEST_CASE("_mbstowcs_l delegates to mbstowcs")
{
  wchar_t a[kSlots] = {};
  wchar_t b[kSlots] = {};
  REQUIRE(_mbstowcs_l(a, kMixed, kMixedLen, nullptr) ==
          mbstowcs(b, kMixed, kMixedLen));
  REQUIRE(a[0] == L'a');
  REQUIRE(a[1] == 0x3042);
  REQUIRE(a[2] == 0xD83D);
  REQUIRE(a[3] == 0xDE00);
  REQUIRE(a[4] == L'z');
  REQUIRE(a[5] == 0);
  for (size_t i = 0; i < kSlots; i++)
    REQUIRE(a[i] == b[i]);

  // A counting conversion: null destination, big count.
  REQUIRE(_mbstowcs_l(nullptr, kMixed, kMixedLen, nullptr) == kMixedWc);

  // Truncation by count, as mbstowcs does: n counts wide characters, so a
  // partial count is returned rather than (size_t)-1.
  wchar_t c[2] = {};
  REQUIRE(_mbstowcs_l(c, kMixed, 2, nullptr) == 2);
  REQUIRE(c[0] == L'a');
  REQUIRE(c[1] == 0x3042);

  // No room at all.
  REQUIRE(_mbstowcs_l(c, kMixed, 0, nullptr) == 0);

  // An unconvertible byte reports (size_t)-1.
  REQUIRE(_mbstowcs_l(a, "\xFF", 1, nullptr) == (size_t)-1);
}

TEST_CASE("_mbstowcs_s_l delegates to mbstowcs_s")
{
  // mbstowcs_s's *conv counts the terminator too, so a full conversion of
  // kMixed reports kMixedWc + 1.
  size_t conv = 0xDEAD;
  wchar_t dst[kSlots] = {};
  REQUIRE(_mbstowcs_s_l(&conv, dst, kSlots, kMixed, kMixedLen, nullptr) == 0);
  REQUIRE(conv == kSlots);
  REQUIRE(dst[0] == L'a');
  REQUIRE(dst[1] == 0x3042);
  REQUIRE(dst[2] == 0xD83D);
  REQUIRE(dst[3] == 0xDE00);
  REQUIRE(dst[4] == L'z');
  REQUIRE(dst[5] == 0);

  // A destination too small is a buffer-overrun report.  mbstowcs_s zeroes
  // both the destination and the count on that path, which is the
  // C11-consistent shape the M7 shell chose over wine's stale value.
  wchar_t small[3] = {0xAAAA, 0xAAAA, 0xAAAA};
  conv = 0xDEAD;
  REQUIRE(_mbstowcs_s_l(&conv, small, 3, kMixed, kMixedLen, nullptr) == ERANGE);
  REQUIRE(conv == 0);
  REQUIRE(small[0] == 0);

  // A null buffer with a non-zero size is the same report.
  conv = 0xDEAD;
  REQUIRE(_mbstowcs_s_l(&conv, nullptr, 8, kMixed, kMixedLen, nullptr) ==
          EINVAL);
  REQUIRE(conv == 0);

  // A null conversion pointer is allowed; it just suppresses the count.
  wchar_t all[kSlots] = {};
  REQUIRE(_mbstowcs_s_l(nullptr, all, kSlots, kMixed, kMixedLen, nullptr) == 0);
  REQUIRE(all[5] == 0);

  // An unconvertible source byte is the same report as mbstowcs_s gives.
  conv = 0xDEAD;
  wchar_t one[2] = {0xAAAA, 0xAAAA};
  errno = 0;
  REQUIRE(_mbstowcs_s_l(&conv, one, 2, "\xFF", 1, nullptr) == EILSEQ);
  REQUIRE(conv == 0);
  REQUIRE(one[0] == 0);
  REQUIRE(errno == EILSEQ);
}
