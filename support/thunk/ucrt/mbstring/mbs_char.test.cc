#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <string.h>

// The g4 single-character faces are declared here rather than taken from
// <mbstring.h>/<mbctype.h>: the headers warn that _mbccpy is deprecated,
// and the secure form is not declared by any header at all.  Types must
// match the headers exactly (M9 lesson) -- _locale_t is not void*.  Every
// non-ASCII fixture is a byte array, never a string literal with hex
// escapes, for the reason mbs_copy_case.test.cc spells out.
extern "C"
{
  unsigned int __cdecl _mbctolower(unsigned int c);
  unsigned int __cdecl _mbctolower_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbctoupper(unsigned int c);
  unsigned int __cdecl _mbctoupper_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbctohira(unsigned int c);
  unsigned int __cdecl _mbctohira_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbctokata(unsigned int c);
  unsigned int __cdecl _mbctokata_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbbtombc(unsigned int c);
  unsigned int __cdecl _mbbtombc_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbctombb(unsigned int c);
  unsigned int __cdecl _mbctombb_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbcjistojms(unsigned int c);
  unsigned int __cdecl _mbcjistojms_l(unsigned int c, _locale_t locale);
  unsigned int __cdecl _mbcjmstojis(unsigned int c);
  unsigned int __cdecl _mbcjmstojis_l(unsigned int c, _locale_t locale);
  size_t __cdecl _mbclen(const unsigned char *string);
  size_t __cdecl _mbclen_l(const unsigned char *string, _locale_t locale);
  void __cdecl _mbccpy(unsigned char *dst, const unsigned char *src);
  void __cdecl
  _mbccpy_l(unsigned char *dst, const unsigned char *src, _locale_t locale);
  errno_t __cdecl _mbccpy_s(unsigned char *dst,
                            size_t size,
                            int *pcopied,
                            const unsigned char *src);
  errno_t __cdecl _mbccpy_s_l(unsigned char *dst,
                              size_t size,
                              int *pcopied,
                              const unsigned char *src,
                              _locale_t locale);
} // extern "C"

namespace
{
  const unsigned char kHira[] = {0xE3, 0x81, 0x82, 0};        // U+3042
  const unsigned char kEmoji[] = {0xF0, 0x9F, 0x98, 0x80, 0}; // U+1F600
  const unsigned char kTwo[] = {0xC3, 0xA9, 0};               // U+00E9
  const unsigned char kTrunc[] = {0xE3, 0};          // a lead cut by NUL
  const unsigned char kTrail[] = {0x81, 0};          // a stray trail byte
  const unsigned char kOverlong[] = {0xC0, 0x80, 0}; // an overlong form
} // namespace

// ------------------------------------------------------------------- fold

TEST_CASE("_mbctolower folds a whole codepoint")
{
  // ASCII is the byte table; above it the fold is the codepoint folder,
  // which is why Latin-1, Greek and fullwidth move while the kana blocks
  // do not.  Each value here was read off the engine through a probe,
  // not assumed from the Unicode data files.
  REQUIRE(_mbctolower('A') == 'a');
  REQUIRE(_mbctolower('a') == 'a');
  REQUIRE(_mbctolower('0') == '0');

  REQUIRE(_mbctolower(0x00C0) == 0x00E0); // A-grave
  REQUIRE(_mbctolower(0x00C9) == 0x00E9); // E-acute
  REQUIRE(_mbctolower(0x00E0) == 0x00E0); // already lower: unchanged
  REQUIRE(_mbctolower(0x00DF) == 0x00DF); // sharp s has no upper case
  REQUIRE(_mbctolower(0x0391) == 0x03B1); // Greek capital alpha
  REQUIRE(_mbctolower(0xFF21) == 0xFF41); // fullwidth A

  // The invariant fold does not move kana, CJK or anything above the
  // BMP, so these are the values that prove the argument really is a
  // whole codepoint and not a DBCS pair.
  REQUIRE(_mbctolower(0x3042) == 0x3042);
  REQUIRE(_mbctolower(0x30A2) == 0x30A2);
  REQUIRE(_mbctolower(0x4E00) == 0x4E00);
  REQUIRE(_mbctolower(0x1F600) == 0x1F600);
}

TEST_CASE("_mbctoupper is the other direction of the same table")
{
  REQUIRE(_mbctoupper('a') == 'A');
  REQUIRE(_mbctoupper('A') == 'A');
  REQUIRE(_mbctoupper(0x00E0) == 0x00C0);
  REQUIRE(_mbctoupper(0x00E9) == 0x00C9);
  REQUIRE(_mbctoupper(0x00C0) == 0x00C0);
  REQUIRE(_mbctoupper(0x03B1) == 0x0391);
  REQUIRE(_mbctoupper(0xFF41) == 0xFF21);

  REQUIRE(_mbctoupper(0x3042) == 0x3042);
  REQUIRE(_mbctoupper(0x4E00) == 0x4E00);
  REQUIRE(_mbctoupper(0x1F600) == 0x1F600);
}

TEST_CASE("a value that is not a scalar is not folded and does not touch errno")
{
  // The `unsigned int` argument can hold values that are not characters
  // at all.  The fold has nothing to say about them -- and nothing to
  // report either: the reference's own "no mapping" exits leave errno
  // alone, so a caller's errno survives a call that does nothing.
  errno = ERANGE;
  REQUIRE(_mbctolower(0xD800) == 0xD800);     // a surrogate
  REQUIRE(_mbctoupper(0xDFFF) == 0xDFFF);     // the other end
  REQUIRE(_mbctolower(0x110000) == 0x110000); // past the last scalar
  REQUIRE(_mbctoupper(0xFFFFFFFFu) == 0xFFFFFFFFu);
  REQUIRE(errno == ERANGE);
}

TEST_CASE("the fold faces ignore a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbctolower_l(0x00C0, anywhere) == 0x00E0);
  REQUIRE(_mbctoupper_l(0x00E0, anywhere) == 0x00C0);
  REQUIRE(_mbctolower_l(0x3042, anywhere) == 0x3042);
  REQUIRE(_mbctoupper_l(0xD800, anywhere) == 0xD800);
}

// ------------------------------------------------------------------- kana

TEST_CASE("_mbctohira and _mbctokata shift the kana blocks by one 0x60")
{
  // The two blocks are the same length and line up exactly, so the
  // mapping is a single offset -- and the reference's Shift-JIS
  // arithmetic is not it (0x30A2 - 0xA1 would be U+3001, not hiragana).
  REQUIRE(_mbctokata(0x3042) == 0x30A2);
  REQUIRE(_mbctohira(0x30A2) == 0x3042);

  // Both ends of both ranges, so an off-by-one at either boundary shows.
  REQUIRE(_mbctokata(0x3041) == 0x30A1);
  REQUIRE(_mbctokata(0x3096) == 0x30F6);
  REQUIRE(_mbctohira(0x30A1) == 0x3041);
  REQUIRE(_mbctohira(0x30F6) == 0x3096);

  // One past each range, the long vowel mark that sits between them, and
  // characters from other scripts are all left alone.
  REQUIRE(_mbctokata(0x3097) == 0x3097);
  REQUIRE(_mbctohira(0x30F7) == 0x30F7);
  REQUIRE(_mbctokata(0x30FC) == 0x30FC);
  REQUIRE(_mbctohira(0x30FC) == 0x30FC);
  REQUIRE(_mbctokata('A') == 'A');
  REQUIRE(_mbctohira(0x4E00) == 0x4E00);
}

TEST_CASE("the kana round trip is exact on kana and a no-op off it")
{
  for (unsigned int c = 0x3041; c <= 0x3096; ++c) {
    INFO("U+30" << std::hex << c);
    REQUIRE(_mbctohira(_mbctokata(c)) == c);
  }
  for (unsigned int c = 0x30A1; c <= 0x30F6; ++c) {
    INFO("U+30" << std::hex << c);
    REQUIRE(_mbctokata(_mbctohira(c)) == c);
  }
  // Off the blocks neither direction moves, so the pair commutes with
  // nothing -- but must at least be a no-op.
  for (unsigned int c = 0x3000; c < 0x3100; ++c) {
    if ((c >= 0x3041 && c <= 0x3096) || (c >= 0x30A1 && c <= 0x30F6))
      continue;
    REQUIRE(_mbctohira(c) == c);
    REQUIRE(_mbctokata(c) == c);
  }
}

TEST_CASE("the kana faces ignore a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbctokata_l(0x3042, anywhere) == 0x30A2);
  REQUIRE(_mbctohira_l(0x30A2, anywhere) == 0x3042);
}

// -------------------------------------------------------- shift-jis only

TEST_CASE("the Shift-JIS-only faces are identity and leave errno alone")
{
  // All four guard on the kanji codepage, which is never this layer's,
  // so the reference's own early return is the whole answer.  The
  // sample spans the values the reference would have converted (its
  // cp932 branches) and values it would have refused with EILSEQ.
  const unsigned int sample[] = {
      0x41,
      0x20,
      0x30,
      0xA1,
      0xDF,
      0x8140,
      0x829F,
      0xE040,
      0x21,
      0x7E,
      0x9F,
      0xFFFF,
      0x3042,
      0x1F600,
  };
  for (unsigned int c : sample) {
    INFO("U+" << std::hex << c);
    errno = ERANGE;
    REQUIRE(_mbbtombc(c) == c);
    REQUIRE(_mbctombb(c) == c);
    REQUIRE(_mbcjistojms(c) == c);
    REQUIRE(_mbcjmstojis(c) == c);
    REQUIRE(errno == ERANGE);
  }
}

TEST_CASE("the Shift-JIS faces ignore a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbbtombc_l(0x41, anywhere) == 0x41);
  REQUIRE(_mbctombb_l(0x829F, anywhere) == 0x829F);
  REQUIRE(_mbcjistojms_l(0x21, anywhere) == 0x21);
  REQUIRE(_mbcjmstojis_l(0xE040, anywhere) == 0xE040);
}

// ------------------------------------------------------------------ length

TEST_CASE("_mbclen answers the whole character's width")
{
  REQUIRE(_mbclen((const unsigned char *)"a") == 1);
  REQUIRE(_mbclen(kTwo) == 2);
  REQUIRE(_mbclen(kHira) == 3);
  REQUIRE(_mbclen(kEmoji) == 4);

  // A byte that begins no complete well-formed character is a single
  // byte's worth: a cut lead, a stray trail byte, an overlong form.
  REQUIRE(_mbclen(kTrunc) == 1);
  REQUIRE(_mbclen(kTrail) == 1);
  REQUIRE(_mbclen(kOverlong) == 1);
  REQUIRE(_mbclen((const unsigned char *)"") == 1);

  // The reference dereferences without checking; this answers 0 and
  // EINVAL instead, which is the hardening the whole layer applies.
  errno = 0;
  REQUIRE(_mbclen(nullptr) == 0);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbclen ignores a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbclen_l(kHira, anywhere) == 3);
  REQUIRE(_mbclen_l(nullptr, anywhere) == 0);
}

// -------------------------------------------------------------------- copy

TEST_CASE("_mbccpy copies one whole character")
{
  unsigned char dst[8];
  memset(dst, 'x', sizeof dst);

  _mbccpy(dst, kHira);
  REQUIRE(memcmp(dst, kHira, 3) == 0);
  REQUIRE(dst[3] == 'x'); // the character alone: no terminator written

  memset(dst, 'x', sizeof dst);
  _mbccpy(dst, (const unsigned char *)"a");
  REQUIRE(dst[0] == 'a');
  REQUIRE(dst[1] == 'x');

  // A cut lead copies only the terminator, the reference's repair.
  memset(dst, 'x', sizeof dst);
  _mbccpy(dst, kTrunc);
  REQUIRE(dst[0] == 0);
  REQUIRE(dst[1] == 'x');
}

TEST_CASE("_mbccpy_s is the reference's validation order")
{
  unsigned char dst[8];

  // Success, and the byte count the reference reports.
  int copied = -1;
  memset(dst, 'x', sizeof dst);
  REQUIRE(_mbccpy_s(dst, sizeof dst, &copied, kHira) == 0);
  REQUIRE(copied == 3);
  REQUIRE(memcmp(dst, kHira, 3) == 0);
  REQUIRE(dst[3] == 'x');

  // A null source is checked after the destination and answers EINVAL
  // with the destination terminated.
  memset(dst, 'x', sizeof dst);
  copied = -1;
  errno = 0;
  REQUIRE(_mbccpy_s(dst, sizeof dst, &copied, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(dst[0] == 0);
  REQUIRE(copied == 0);

  // ... and the destination itself is validated first of all.
  copied = -1;
  errno = 0;
  REQUIRE(_mbccpy_s(nullptr, sizeof dst, &copied, kHira) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(copied == 0);
  errno = 0;
  REQUIRE(_mbccpy_s(dst, 0, &copied, kHira) == EINVAL);
  REQUIRE(errno == EINVAL);

  // A destination too small for the character is ERANGE, not EILSEQ:
  // the source is well formed and the buffer is at fault.
  memset(dst, 'x', sizeof dst);
  copied = -1;
  errno = 0;
  REQUIRE(_mbccpy_s(dst, 3, &copied, kEmoji) == ERANGE);
  REQUIRE(errno == ERANGE);
  REQUIRE(dst[0] == 0);
  REQUIRE(copied == 0);

  // A size that is exactly the character's width succeeds and writes no
  // terminator -- there is no room for one, and the reference's own
  // success writes none either.
  memset(dst, 'x', sizeof dst);
  copied = -1;
  REQUIRE(_mbccpy_s(dst, 4, &copied, kEmoji) == 0);
  REQUIRE(copied == 4);
  REQUIRE(memcmp(dst, kEmoji, 4) == 0);

  // A source ending in half a character is EILSEQ, and only the
  // terminator is written.
  memset(dst, 'x', sizeof dst);
  copied = -1;
  errno = 0;
  REQUIRE(_mbccpy_s(dst, sizeof dst, &copied, kTrunc) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  REQUIRE(dst[0] == 0);
  REQUIRE(copied == 1);
  REQUIRE(dst[1] == 'x');

  // The byte count is optional.
  REQUIRE(_mbccpy_s(dst, sizeof dst, nullptr, kHira) == 0);
  REQUIRE(memcmp(dst, kHira, 3) == 0);
}

TEST_CASE("the copy faces ignore a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  unsigned char dst[8];
  memset(dst, 'x', sizeof dst);
  _mbccpy_l(dst, kHira, anywhere);
  REQUIRE(memcmp(dst, kHira, 3) == 0);

  int copied = -1;
  REQUIRE(_mbccpy_s_l(dst, sizeof dst, &copied, kEmoji, anywhere) == 0);
  REQUIRE(copied == 4);
  REQUIRE(memcmp(dst, kEmoji, 4) == 0);
}
