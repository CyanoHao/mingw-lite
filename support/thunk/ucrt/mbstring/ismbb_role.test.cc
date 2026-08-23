#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>

#include "mbs_test_matrix.h"

// mbctype.h declares the _ismbb* and mbstring.h the role faces, all
// with __declspec(dllimport); re-declaring them is what every other
// ucrt test in this tree does (M12 finding).
extern "C"
{
  int __cdecl _ismbbalnum(unsigned int c);
  int __cdecl _ismbbalpha(unsigned int c);
  int __cdecl _ismbbblank(unsigned int c);
  int __cdecl _ismbbgraph(unsigned int c);
  int __cdecl _ismbbkalnum(unsigned int c);
  int __cdecl _ismbbkana(unsigned int c);
  int __cdecl _ismbbkprint(unsigned int c);
  int __cdecl _ismbbkpunct(unsigned int c);
  int __cdecl _ismbblead(unsigned int c);
  int __cdecl _ismbbprint(unsigned int c);
  int __cdecl _ismbbpunct(unsigned int c);
  int __cdecl _ismbbtrail(unsigned int c);
  int __cdecl _ismbblead_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbbtrail_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbbkana_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbbprint_l(unsigned int c, _locale_t locale);

  int __cdecl _mbbtype(unsigned char c, int ctype);
  int __cdecl _mbbtype_l(unsigned char c, int ctype, _locale_t locale);
  int __cdecl _mbsbtype(const unsigned char *s, size_t pos);
  int __cdecl _mbsbtype_l(const unsigned char *s, size_t pos, _locale_t locale);

  int __cdecl _ismbslead(const unsigned char *s, const unsigned char *current);
  int __cdecl _ismbstrail(const unsigned char *s, const unsigned char *current);
  int __cdecl _ismbslead_l(const unsigned char *s,
                           const unsigned char *current,
                           _locale_t locale);
  int __cdecl _ismbstrail_l(const unsigned char *s,
                            const unsigned char *current,
                            _locale_t locale);
} // extern "C"

// mbctype.h values, spelled out so the test does not depend on which of
// them the i686 headers happen to expose.  Note the ctype argument of
// _mbbtype is a _MBC_* *return* value, not one of the _MS/_MP/_M1/_M2
// table bits.
namespace
{
  constexpr int MBC_ILLEGAL = -1;
  constexpr int MBC_SINGLE = 0;
  constexpr int MBC_LEAD = 1;
  constexpr int MBC_TRAIL = 2;

  // plan-3 D8a: the UTF-8 byte roles, independent of the reference DBCS
  // table.  lead = 0xC2..0xF4, trail = 0x80..0xBF.
  bool is_lead(unsigned b)
  {
    return b >= 0xC2 && b <= 0xF4;
  }
  bool is_trail(unsigned b)
  {
    return b >= 0x80 && b <= 0xBF;
  }
  bool ascii_print(unsigned b)
  {
    return b >= 0x20 && b < 0x7F;
  }
  bool is_control(unsigned b)
  {
    return b < 0x20 || b == 0x7F;
  }
} // namespace

TEST_CASE("_ismbb* full 256-byte role oracle")
{
  for (unsigned b = 0; b < 256; b++) {
    INFO("byte " << b);

    const bool alpha = (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z');
    const bool digit = b >= '0' && b <= '9';
    const bool space = b == ' ' || (b >= '\t' && b <= '\r');
    const bool blank = b == ' ' || b == '\t';
    const bool ascii = b < 0x80;
    const bool print = ascii && !is_control(b);
    const bool graph = print && !space;
    const bool punct = ascii && print && !space && !alpha && !digit;

    // Byte role (D8j: the ASCII predicates go false at 0x80).
    REQUIRE(_ismbblead(b) == (is_lead(b) ? 1 : 0));
    REQUIRE(_ismbbtrail(b) == (is_trail(b) ? 1 : 0));
    REQUIRE(_ismbbalpha(b) == (alpha ? 1 : 0));
    REQUIRE(_ismbbalnum(b) == ((alpha || digit) ? 1 : 0));
    REQUIRE(_ismbbprint(b) == (print ? 1 : 0));
    REQUIRE(_ismbbgraph(b) == (graph ? 1 : 0));
    REQUIRE(_ismbbblank(b) == (blank ? 1 : 0));
    REQUIRE(_ismbbpunct(b) == (punct ? 1 : 0));

    // Single-byte katakana does not exist in UTF-8 (D8i).
    REQUIRE(_ismbbkana(b) == 0);
    REQUIRE(_ismbbkalnum(b) == 0);
    REQUIRE(_ismbbkprint(b) == 0);
    REQUIRE(_ismbbkpunct(b) == 0);
  }
}

TEST_CASE("_ismbb* lead and trail never overlap")
{
  for (unsigned b = 0; b < 256; b++) {
    if (_ismbblead(b))
      REQUIRE(_ismbbtrail(b) == 0);
    if (_ismbbtrail(b))
      REQUIRE(_ismbblead(b) == 0);
  }

  // The _ismbb* role predicates and the _ismbs* string-context ones
  // answer different questions: 0xA1 is a *continuation byte* here even
  // though it is a Shift-JIS katakana lead there (D8i note).
  REQUIRE(_ismbbtrail(0xA1) == 1);
  REQUIRE(_ismbbkana(0xA1) == 0);
}

TEST_CASE("_ismbb* out-of-range inputs")
{
  for (unsigned c : {0x100u, 0x1000u, 0x4E00u, 0xFFFFFFFFu}) {
    REQUIRE(_ismbbalnum(c) == 0);
    REQUIRE(_ismbbalpha(c) == 0);
    REQUIRE(_ismbbblank(c) == 0);
    REQUIRE(_ismbbgraph(c) == 0);
    REQUIRE(_ismbbprint(c) == 0);
    REQUIRE(_ismbbpunct(c) == 0);
    REQUIRE(_ismbblead(c) == 0);
    REQUIRE(_ismbbtrail(c) == 0);
    REQUIRE(_ismbbkana(c) == 0);
  }
}

TEST_CASE("_ismbb* _l twins ignore the locale")
{
  REQUIRE(_ismbblead_l(0xE3, nullptr) == 1);
  REQUIRE(_ismbblead_l(0x41, nullptr) == 0);
  REQUIRE(_ismbbtrail_l(0x81, nullptr) == 1);
  REQUIRE(_ismbbkana_l(0xA1, nullptr) == 0);
  REQUIRE(_ismbbprint_l('a', nullptr) == 1);
}

TEST_CASE("_mbbtype state machine")
{
  // The reference switch has exactly one special case: a *previous* byte
  // that was reported as _MBC_LEAD turns the next byte into _MBC_TRAIL or
  // _MBC_ILLEGAL.  Every other previous state takes the same branch.
  for (int prev : {MBC_SINGLE, MBC_TRAIL, MBC_ILLEGAL, 42, -7}) {
    for (unsigned b = 0; b < 256; b++) {
      INFO("byte " << b << " after state " << prev);
      const int got = _mbbtype((unsigned char)b, prev);
      if (is_lead(b)) {
        REQUIRE(got == MBC_LEAD);
      } else if (ascii_print(b)) {
        REQUIRE(got == MBC_SINGLE);
      } else {
        REQUIRE(got == MBC_ILLEGAL);
      }
    }
  }

  // In the lead state only a continuation byte continues.
  for (unsigned b = 0; b < 256; b++) {
    INFO("byte " << b << " after a lead");
    REQUIRE(_mbbtype((unsigned char)b, MBC_LEAD) ==
            (is_trail(b) ? MBC_TRAIL : MBC_ILLEGAL));
  }

  // The two states agree on a well-formed two-byte character.
  REQUIRE(_mbbtype(0xC3, MBC_SINGLE) == MBC_LEAD);
  REQUIRE(_mbbtype(0xA9, MBC_LEAD) == MBC_TRAIL);
  REQUIRE(_mbbtype(0xA9, MBC_TRAIL) == MBC_ILLEGAL);
}

TEST_CASE("_mbbtype_l ignores the locale")
{
  REQUIRE(_mbbtype_l(0xC3, MBC_SINGLE, nullptr) == MBC_LEAD);
  REQUIRE(_mbbtype_l(0xA9, MBC_LEAD, nullptr) == MBC_TRAIL);
  REQUIRE(_mbbtype_l('A', MBC_SINGLE, nullptr) == MBC_SINGLE);
  REQUIRE(_mbbtype_l(0x00, MBC_SINGLE, nullptr) == MBC_ILLEGAL);
}

TEST_CASE("_mbsbtype position matrix")
{
  //  'a' + U+3042 (3 bytes) + U+1F600 (4 bytes) + 'z'
  const char *s = "a\xE3\x81\x82\xF0\x9F\x98\x80z";
  const unsigned char *u = (const unsigned char *)s;

  const int want[] = {MBC_SINGLE,
                      MBC_LEAD,
                      MBC_TRAIL,
                      MBC_TRAIL,
                      MBC_LEAD,
                      MBC_TRAIL,
                      MBC_TRAIL,
                      MBC_TRAIL,
                      MBC_SINGLE};
  for (size_t i = 0; i < sizeof(want) / sizeof(want[0]); i++) {
    INFO("offset " << i);
    errno = 0;
    REQUIRE(_mbsbtype(u, i) == want[i]);
    REQUIRE(errno == 0);
  }
}

TEST_CASE("_mbsbtype terminator and out-of-range offsets")
{
  const unsigned char *u = (const unsigned char *)"ab";

  // At and past the terminator: _MBC_ILLEGAL with errno EINVAL (D8b).
  errno = 0;
  REQUIRE(_mbsbtype(u, 2) == MBC_ILLEGAL);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_mbsbtype(u, 99) == MBC_ILLEGAL);
  REQUIRE(errno == EINVAL);

  // A null string is out of range too.
  errno = 0;
  REQUIRE(_mbsbtype(nullptr, 0) == MBC_ILLEGAL);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsbtype on malformed input")
{
  // A bare continuation byte is not a character: illegal, and the walk
  // resynchronises one byte at a time so the string after it still reads.
  const unsigned char *u = (const unsigned char *)"\x81"
                                                  "a";
  errno = 0;
  REQUIRE(_mbsbtype(u, 0) == MBC_ILLEGAL);
  REQUIRE(_mbsbtype(u, 1) == MBC_SINGLE);

  // A truncated sequence: the lead byte is not a complete character.
  const unsigned char *t = (const unsigned char *)"\xE3\x81";
  errno = 0;
  REQUIRE(_mbsbtype(t, 0) == MBC_ILLEGAL);

  // ...and the overlong two-byte form never reads as a character either.
  const unsigned char *o = (const unsigned char *)"\xC0\x80";
  errno = 0;
  REQUIRE(_mbsbtype(o, 0) == MBC_ILLEGAL);
  REQUIRE(_mbsbtype(o, 1) == MBC_ILLEGAL);
}

TEST_CASE("_mbsbtype_l ignores the locale")
{
  const unsigned char *u = (const unsigned char *)"a\xE3\x81\x82";
  REQUIRE(_mbsbtype_l(u, 0, nullptr) == MBC_SINGLE);
  REQUIRE(_mbsbtype_l(u, 1, nullptr) == MBC_LEAD);
  REQUIRE(_mbsbtype_l(u, 2, nullptr) == MBC_TRAIL);
  REQUIRE(_mbsbtype_l(u, 4, nullptr) == MBC_ILLEGAL);
}

TEST_CASE("_ismbslead / _ismbstrail pointer matrix")
{
  const char *s = "a\xE3\x81\x82\xF0\x9F\x98\x80z";
  const unsigned char *u = (const unsigned char *)s;
  const size_t n = 9;

  // lead[]: the first byte of a multi-byte character (D8c).
  const int want_lead[] = {0, 1, 0, 0, 1, 0, 0, 0, 0};
  // trail[]: a continuation byte of one that started earlier.
  const int want_trail[] = {0, 0, 1, 1, 0, 1, 1, 1, 0};

  for (size_t i = 0; i < n; i++) {
    INFO("offset " << i);
    REQUIRE(_ismbslead(u, u + i) == want_lead[i]);
    REQUIRE(_ismbstrail(u, u + i) == want_trail[i]);
  }
}

TEST_CASE("_ismbslead / _ismbstrail out-of-range pointers")
{
  const unsigned char *u = (const unsigned char *)"a\xE3\x81\x82";
  const unsigned char *after = u + 4; // the terminator

  // The terminator is neither a lead nor a trail, and leaves errno alone.
  errno = 0;
  REQUIRE(_ismbslead(u, after) == 0);
  REQUIRE(_ismbstrail(u, after) == 0);
  REQUIRE(errno == 0);

  // A pointer before the start, or a null pair, answers 0.
  REQUIRE(_ismbslead(u, u - 1) == 0);
  REQUIRE(_ismbstrail(u, u - 1) == 0);
  REQUIRE(_ismbslead(nullptr, after) == 0);
  REQUIRE(_ismbstrail(nullptr, after) == 0);
  REQUIRE(_ismbslead(u, nullptr) == 0);
  REQUIRE(_ismbstrail(u, nullptr) == 0);

  // The first byte of the string may be a lead (pos == string start).
  const unsigned char *g = (const unsigned char *)"\xE3\x81\x82";
  REQUIRE(_ismbslead(g, g) == 1);
  REQUIRE(_ismbstrail(g, g) == 0);
}

TEST_CASE("_ismbslead / _ismbstrail on malformed input")
{
  // 0x81 with no lead in front is a continuation byte of nothing.
  const unsigned char *u = (const unsigned char *)"a\x81";
  REQUIRE(_ismbslead(u, u + 1) == 0);
  REQUIRE(_ismbstrail(u, u + 1) == 0);

  // A truncated sequence is not a well-formed character, so its lead
  // byte does not count as a lead and its continuation bytes do not
  // count as trails.
  const unsigned char *t = (const unsigned char *)"\xE3\x81";
  REQUIRE(_ismbslead(t, t) == 0);
  REQUIRE(_ismbstrail(t, t + 1) == 0);

  // The encoded-surrogate form is not a character either.
  const unsigned char *s = (const unsigned char *)"\xED\xA0\x80";
  REQUIRE(_ismbslead(s, s) == 0);
  REQUIRE(_ismbstrail(s, s + 1) == 0);
  REQUIRE(_ismbstrail(s, s + 2) == 0);
}

TEST_CASE("_ismbs* _l twins ignore the locale")
{
  const unsigned char *u = (const unsigned char *)"a\xE3\x81\x82";
  REQUIRE(_ismbslead_l(u, u + 1, nullptr) == 1);
  REQUIRE(_ismbslead_l(u, u + 2, nullptr) == 0);
  REQUIRE(_ismbstrail_l(u, u + 2, nullptr) == 1);
  REQUIRE(_ismbstrail_l(u, u, nullptr) == 0);
}
