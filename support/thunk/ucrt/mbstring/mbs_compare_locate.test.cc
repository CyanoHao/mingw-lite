#include <catch_amalgamated.hpp>

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <string.h>

#include "mbs_str_decls.h"

// plan-3 §5.3 line b, second third: the twelve comparison faces and the
// seven locate faces.  Both groups are one engine call, so the tests are
// written as contrasts -- a pair of faces that must differ, and a pair
// that must not -- rather than as a table of single answers.

namespace
{
  // Every non-ASCII fixture below is written as bytes, never as a string
  // literal with hex escapes.  A C hex escape is greedy over the
  // characters that follow it, so "\xE3\x81\x82cd" is not "a three-byte
  // character then c then d" -- it is one out-of-range escape, and the
  // compiler says so only with a warning.  Writing the bytes makes the
  // fixture say what it means.
  //
  // U+00C0 A-grave and U+00E9 e-acute, upper; their lower partners.  In
  // UTF-8 these are C3 80 / C3 A9 and C3 A0 / C3 89 -- not the Latin-1
  // bytes C0 and E9, which are not decodable at all and which this layer
  // therefore reports as EILSEQ.  Two bytes each, so the byte-counted
  // faces see whole characters.
  const unsigned char kUpperLatin[] = {0xC3, 0x80, 0xC3, 0xA9, 0};
  const unsigned char kLowerLatin[] = {0xC3, 0xA0, 0xC3, 0x89, 0};

  // U+FF21 / U+FF41, fullwidth A upper and lower: three bytes on both
  // sides of the fold, which is the widest pair the invariant fold
  // actually moves.  U+FF41's third byte is 0x81, not 0xB1 -- 0xB1 would
  // make it U+FF71, a halfwidth katakana, which is not a case pair at all.
  const unsigned char kFullUpper[] = {0xEF, 0xBC, 0xA1, 0};
  const unsigned char kFullLower[] = {0xEF, 0xBD, 0x81, 0};

  // U+3042 hiragana A and U+30A2 katakana A.  These are the pair a
  // Japanese locale would fold together and LCMapStringW's invariant
  // table does not, verified against the product: towlower(0x30A2) is
  // 0x30A2.  Every case assertion in this layer therefore has to treat
  // kana as two unrelated code points, and saying so in a test is what
  // keeps that from being an accident later.
  const unsigned char kHira[] = {0xE3, 0x81, 0x82, 0};
  const unsigned char kKata[] = {0xE3, 0x82, 0xA2, 0};
  const unsigned char kHira2[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 0};
  const unsigned char kHira2x[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 'x', 0};

  // 'a' + U+3042 + 'z', and the same shape with a second U+3042.
  const unsigned char kAHZ[] = {'a', 0xE3, 0x81, 0x82, 'z', 0};
  const unsigned char kAHZ2[] = {
      'a', 0xE3, 0x81, 0x82, 'z', 0xE3, 0x81, 0x82, 0};

  // "ab" + U+3042 + "cd" -- the string the four scan faces share.  Seven
  // bytes, and the offsets below are the whole point of naming them.
  const unsigned char kScan[] = {'a', 'b', 0xE3, 0x81, 0x82, 'c', 'd', 0};
  constexpr size_t kScanLen = 7;

  // The two Latin-1 letters on their own, for the face-by-byte-bound
  // arithmetic: U+00C0 and U+00E9, two bytes each.
  const unsigned char kAGrave[] = {0xC3, 0x80, 0};
  const unsigned char kEAcute[] = {0xC3, 0xA9, 0};

  const unsigned char kAbc[] = "abc";
  const unsigned char kAbc2[] = "abcabc";

  // 'a' followed by a three-byte lead the terminator cuts short.  Named
  // because a string literal's address is not something to compare a
  // result against.
  const unsigned char kCut[] = {'a', 0xE3, 0};
} // namespace

// ------------------------------------------------------------- comparison

TEST_CASE("_mbscmp orders by code point and normalises to -1/0/1")
{
  REQUIRE(_mbscmp(kAbc, kAbc) == 0);
  REQUIRE(_mbscmp((const unsigned char *)"a", (const unsigned char *)"b") ==
          -1);
  REQUIRE(_mbscmp((const unsigned char *)"b", (const unsigned char *)"a") == 1);

  // A prefix is less than what extends it: the terminator compares below
  // every character.
  REQUIRE(_mbscmp((const unsigned char *)"abc",
                  (const unsigned char *)"abcd") == -1);
  REQUIRE(_mbscmp((const unsigned char *)"abcd",
                  (const unsigned char *)"abc") == 1);
  REQUIRE(_mbscmp(kHira, kHira2) == -1);
  REQUIRE(_mbscmp(kHira2, kHira) == 1);

  // Code-point order and UTF-8 byte order agree by construction, so this
  // is the same answer strcmp would give -- which is the point: the
  // engine decodes to order, it does not depend on the encoding to.
  REQUIRE(_mbscmp(kAHZ, kAbc) == 1);
  REQUIRE(_mbscmp(kHira, kKata) == -1); // 0x3042 < 0x30A2
}

TEST_CASE("_mbscmp never mistakes a broken byte for the terminator")
{
  // plan-3 D9f, and the one place the compare faces deliberately differ
  // from the reference.  The reference's loop leaves `c` at 0 for a lead
  // byte the terminator follows, which reports "a\xE3" as equal to "a" --
  // the truncated tail reads as if the string ended there.  The engine
  // advances one byte and contributes the raw 0xE3 instead, so the tail
  // is ordered after every real character and the strings are correctly
  // reported as different.
  REQUIRE(_mbscmp((const unsigned char *)"a\xE3", (const unsigned char *)"a") ==
          1);
  REQUIRE(_mbscmp((const unsigned char *)"a", (const unsigned char *)"a\xE3") ==
          -1);
  REQUIRE(_mbscmp((const unsigned char *)"a\xE3",
                  (const unsigned char *)"a\xE3") == 0);

  // A stray continuation byte is a character of its own too, by the same
  // rule, so "a\x81z" is a three-character string.
  REQUIRE(_mbscmp(kAHZ, (const unsigned char *)"a\x81z") == 1);
}

TEST_CASE("_mbscmp on a null operand is EINVAL and _NLSCMPERROR")
{
  errno = 0;
  REQUIRE(_mbscmp(nullptr, kAbc) == INT_MIN);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbscmp(kAbc, nullptr) == INT_MIN);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsncmp counts characters, _mbsnbcmp counts bytes")
{
  // The family's own split, on the same two strings and the same numbers.
  // Counted in characters the first hiragana is compared; counted in
  // bytes a one-byte bound cannot hold a three-byte character, so both
  // sides read as terminated and the answer is 0.
  REQUIRE(_mbsncmp(kHira, kKata, 1) == -1);
  REQUIRE(_mbsnbcmp(kHira, kKata, 1) == 0);

  // Latin-1 keeps the split visible one byte later: a two-byte bound is
  // still not enough for the byte-counted face, but is enough for the
  // character-counted one.
  REQUIRE(_mbsnbcmp(kUpperLatin, kLowerLatin, 1) == 0);
  REQUIRE(_mbsnbcmp(kUpperLatin, kLowerLatin, 2) == -1);
  REQUIRE(_mbsncmp(kUpperLatin, kLowerLatin, 2) == -1);

  // Three bytes is exactly one U+3042, so the byte-counted face finally
  // has a character to compare.
  REQUIRE(_mbsnbcmp(kHira, kKata, 3) == -1);
  REQUIRE(_mbsncmp(kHira, kKata, 3) == -1);
  REQUIRE(_mbsnbcmp(kHira, kKata, 4) == -1);
}

TEST_CASE("the counted faces stop at the shorter string, not the bound")
{
  REQUIRE(_mbsncmp(kHira2, kHira2x, 2) == 0);
  REQUIRE(_mbsncmp(kHira2, kHira2x, 3) == -1);
  REQUIRE(_mbsncmp(kHira2, kHira2x, 99) == -1);
  REQUIRE(_mbsncmp((const unsigned char *)"abcd",
                   (const unsigned char *)"abxx",
                   2) == 0);
  REQUIRE(_mbsncmp((const unsigned char *)"abcd",
                   (const unsigned char *)"abxx",
                   3) == -1);
  REQUIRE(_mbsnbcmp((const unsigned char *)"abcd",
                    (const unsigned char *)"abxx",
                    2) == 0);
  REQUIRE(_mbsnbcmp((const unsigned char *)"abcd",
                    (const unsigned char *)"abxx",
                    3) == -1);
}

TEST_CASE("a zero count answers 0 before the pointers are looked at")
{
  // The reference's `if (!n) return 0` sits ahead of its validation, and
  // so does this.  A caller that passes a null string with a zero count is
  // asking for nothing; the error would be about an argument it never
  // used.
  errno = 0;
  REQUIRE(_mbsncmp(nullptr, nullptr, 0) == 0);
  REQUIRE(errno == 0);
  REQUIRE(_mbsncmp(nullptr, kAbc, 0) == 0);
  REQUIRE(_mbsnbcmp(kAbc, nullptr, 0) == 0);
  REQUIRE(errno == 0);

  errno = 0;
  REQUIRE(_mbsnicmp(nullptr, kAbc, 0) == 0);
  REQUIRE(_mbsnbicmp(kAbc, nullptr, 0) == 0);
  REQUIRE(errno == 0);
}

TEST_CASE("a null operand with a live count is EINVAL and _NLSCMPERROR")
{
  errno = 0;
  REQUIRE(_mbsncmp(nullptr, kAbc, 1) == INT_MIN);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsncmp(kAbc, nullptr, 1) == INT_MIN);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnbcmp(nullptr, kAbc, 1) == INT_MIN);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnicmp(kAbc, nullptr, 1) == INT_MIN);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnbicmp(nullptr, nullptr, 1) == INT_MIN);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsicmp folds by code point, over the folds that exist")
{
  REQUIRE(_mbsicmp((const unsigned char *)"ABC",
                   (const unsigned char *)"abc") == 0);
  REQUIRE(_mbsicmp(kUpperLatin, kLowerLatin) == 0);
  REQUIRE(_mbsicmp(kFullUpper, kFullLower) == 0);
  REQUIRE(_mbsicmp(kFullUpper, kFullLower) == _mbsicmp(kFullLower, kFullUpper));

  // A fold pair that is still different after folding stays different,
  // and orders by the folded value: 0xE0 < 0xE9.
  REQUIRE(_mbsicmp(kAGrave, kEAcute) == -1);
  REQUIRE(_mbsicmp(kEAcute, kAGrave) == 1);

  // And kana does not fold, in the invariant table this layer reaches
  // through.  A Japanese locale would answer 0 here; the anchor is
  // recorded so the divergence cannot be mistaken for a bug later.
  REQUIRE(_mbsicmp(kHira, kKata) == -1);
  REQUIRE(_mbsicmp(kHira, kKata) == _mbscmp(kHira, kKata));
}

TEST_CASE("the counted icmp faces fold the same way as the whole-string one")
{
  REQUIRE(_mbsnicmp((const unsigned char *)"ABC",
                    (const unsigned char *)"abc",
                    3) == 0);
  REQUIRE(_mbsnicmp((const unsigned char *)"ABC",
                    (const unsigned char *)"abd",
                    2) == 0);
  REQUIRE(_mbsnicmp((const unsigned char *)"ABC",
                    (const unsigned char *)"abd",
                    3) == -1);
  REQUIRE(_mbsnicmp(kUpperLatin, kLowerLatin, 2) == 0);
  REQUIRE(_mbsnicmp(kHira, kKata, 1) == -1);
  REQUIRE(_mbsnicmp(kHira2, kHira2x, 2) == 0);
  REQUIRE(_mbsnicmp(kHira2, kHira2x, 3) == -1);

  // The byte-counted fold keeps the same split as the byte-counted raw
  // compare: one byte cannot hold a whole two-byte character on either
  // side, so the fold can only be seen once a whole character fits, and a
  // whole character each way is enough to answer.  U+00C0 and U+00E9 are
  // two *different* letters -- folding them does not make them equal -- so
  // both tallies answer the same way.
  REQUIRE(_mbsnbicmp(kUpperLatin, kLowerLatin, 1) == 0);
  REQUIRE(_mbsnbicmp(kUpperLatin, kLowerLatin, 2) == 0);
  REQUIRE(_mbsnbicmp(kAGrave, kEAcute, 4) == -1);
  REQUIRE(_mbsnbicmp(kAGrave, kEAcute, 2) == -1);
  REQUIRE(_mbsnbicmp(kHira, kKata, 1) == 0);
  REQUIRE(_mbsnbicmp(kHira, kKata, 3) == -1);
}

TEST_CASE("the coll family mirrors the cmp family, like M8's strcoll")
{
  // plan-3 5.1: there is no collation table in this layer, so a coll face
  // is its cmp twin.  M8 established the precedent for the wide family
  // (strcoll -> strcmp) and the same reasoning applies to the mb one.
  // What the tests check is that the twelve faces really are the six
  // pairs, because a collation face that quietly sorted differently would
  // be the kind of divergence nobody notices until a sort order changes.
  REQUIRE(_mbscoll(kAbc, kAbc) == _mbscmp(kAbc, kAbc));
  REQUIRE(_mbscoll(kAHZ, kAbc) == _mbscmp(kAHZ, kAbc));
  REQUIRE(_mbscoll(kHira, kKata) == _mbscmp(kHira, kKata));
  REQUIRE(
      _mbscoll((const unsigned char *)"a\xE3", (const unsigned char *)"a") ==
      _mbscmp((const unsigned char *)"a\xE3", (const unsigned char *)"a"));
  REQUIRE(_mbsicoll(kUpperLatin, kLowerLatin) ==
          _mbsicmp(kUpperLatin, kLowerLatin));
  REQUIRE(_mbsicoll(kFullUpper, kFullLower) ==
          _mbsicmp(kFullUpper, kFullLower));
  REQUIRE(_mbsicoll(kHira, kKata) == _mbsicmp(kHira, kKata));

  REQUIRE(_mbsncoll(kHira, kKata, 1) == _mbsncmp(kHira, kKata, 1));
  REQUIRE(_mbsncoll(kUpperLatin, kLowerLatin, 2) ==
          _mbsncmp(kUpperLatin, kLowerLatin, 2));
  REQUIRE(
      _mbsnicoll(
          (const unsigned char *)"ABC", (const unsigned char *)"abd", 3) ==
      _mbsnicmp((const unsigned char *)"ABC", (const unsigned char *)"abd", 3));
  REQUIRE(_mbsnbcoll(kHira, kKata, 1) == _mbsnbcmp(kHira, kKata, 1));
  REQUIRE(_mbsnbcoll(kUpperLatin, kLowerLatin, 2) ==
          _mbsnbcmp(kUpperLatin, kLowerLatin, 2));
  REQUIRE(_mbsnbicoll(kHira, kKata, 3) == _mbsnbicmp(kHira, kKata, 3));
}

TEST_CASE("every compare face ignores a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbscmp_l(kAHZ, kAbc, anywhere) == _mbscmp(kAHZ, kAbc));
  REQUIRE(_mbscoll_l(kAHZ, kAbc, anywhere) == _mbscoll(kAHZ, kAbc));
  REQUIRE(_mbsicmp_l(kUpperLatin, kLowerLatin, anywhere) ==
          _mbsicmp(kUpperLatin, kLowerLatin));
  REQUIRE(_mbsicoll_l(kUpperLatin, kLowerLatin, anywhere) ==
          _mbsicoll(kUpperLatin, kLowerLatin));
  REQUIRE(_mbsncmp_l(kHira, kKata, 1, anywhere) == _mbsncmp(kHira, kKata, 1));
  REQUIRE(_mbsncoll_l(kHira, kKata, 1, anywhere) == _mbsncoll(kHira, kKata, 1));
  REQUIRE(_mbsnicmp_l(kUpperLatin, kLowerLatin, 2, anywhere) ==
          _mbsnicmp(kUpperLatin, kLowerLatin, 2));
  REQUIRE(_mbsnicoll_l(kUpperLatin, kLowerLatin, 2, anywhere) ==
          _mbsnicoll(kUpperLatin, kLowerLatin, 2));
  REQUIRE(_mbsnbcmp_l(kHira, kKata, 3, anywhere) == _mbsnbcmp(kHira, kKata, 3));
  REQUIRE(_mbsnbcoll_l(kHira, kKata, 3, anywhere) ==
          _mbsnbcoll(kHira, kKata, 3));
  REQUIRE(_mbsnbicmp_l(kHira, kKata, 3, anywhere) ==
          _mbsnbicmp(kHira, kKata, 3));
  REQUIRE(_mbsnbicoll_l(kHira, kKata, 3, anywhere) ==
          _mbsnbicoll(kHira, kKata, 3));
}

// ----------------------------------------------------------------- locate

TEST_CASE("_mbschr takes a code point, not a byte")
{
  REQUIRE(_mbschr(kAbc2, 'c') == kAbc2 + 2);
  REQUIRE(_mbschr(kAHZ, 0x3042) == kAHZ + 1);
  REQUIRE(_mbschr(kAHZ2, 0x3042) == kAHZ2 + 1); // the *first* one
  REQUIRE(_mbschr(kAbc2, 'd') == nullptr);

  // Searching for the terminator finds the terminator, which is what the
  // reference's DBCS comparison against `c` reduces to when c is 0.
  REQUIRE(_mbschr(kAbc, 0) == kAbc + 3);
  REQUIRE(_mbschr(kAHZ, 0) == kAHZ + 5);

  // A search that cannot be satisfied by any character is a miss even
  // though 0x3042 fits in an unsigned int comfortably.
  REQUIRE(_mbschr(kAbc, 0x3042) == nullptr);
  REQUIRE(_mbschr(kAbc, 0x1F600) == nullptr);

  // plan-3 D9f again: a byte that decodes to nothing stands for itself,
  // so a raw-byte search over a broken string behaves like strchr.
  REQUIRE(_mbschr(kCut, 0xE3) == kCut + 1);
  REQUIRE(_mbschr(kCut, 0x3042) == nullptr);

  errno = 0;
  REQUIRE(_mbschr(nullptr, 'a') == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsrchr finds the last occurrence")
{
  REQUIRE(_mbsrchr(kAbc2, 'c') == kAbc2 + 5);
  REQUIRE(_mbsrchr(kAbc2, 'a') == kAbc2 + 3);
  REQUIRE(_mbsrchr(kAHZ2, 0x3042) == kAHZ2 + 5);
  REQUIRE(_mbsrchr(kAbc2, 'd') == nullptr);
  REQUIRE(_mbsrchr(kAbc, 0) == kAbc + 3);

  // A character the terminator cuts short is not a character, so it is
  // never a hit.  The reference's `else if (!r) r = str` would answer the
  // terminator for a search for that lead byte's code; skipping the dud
  // is the deliberate side of D9l.
  REQUIRE(_mbsrchr((const unsigned char *)"\xE3\x81", 0x3042) == nullptr);

  // Two occurrences, so "last" and "first" are different answers and the
  // test can tell them apart.
  REQUIRE(_mbsrchr(kHira2x, 0x3042) == kHira2x + 3);
  REQUIRE(_mbsrchr(kHira2x, 0x3042) == _mbschr(kHira2x, 0x3042) + 3);

  errno = 0;
  REQUIRE(_mbsrchr(nullptr, 'a') == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsstr matches only at a character boundary")
{
  REQUIRE(_mbsstr(kHira2x, kHira) == kHira2x);
  REQUIRE(_mbsstr(kAHZ2, kHira) == kAHZ2 + 1);
  REQUIRE(_mbsstr(kHira2x, kKata) == nullptr);
  REQUIRE(_mbsstr(kAbc2, (const unsigned char *)"ca") == kAbc2 + 2);
  REQUIRE(_mbsstr(kAbc2, (const unsigned char *)"abcd") == nullptr);

  // The alignment anchor: the needle's bytes are present in the string
  // but start inside a character, so a byte search would find them and
  // this face must not.  strstr over the same pair answers kHira2 + 1.
  const unsigned char *bytewise =
      (const unsigned char *)strstr((const char *)kHira2, "\x81\x82");
  REQUIRE(bytewise == kHira2 + 1);
  REQUIRE(_mbsstr(kHira2, (const unsigned char *)"\x81\x82") == nullptr);
}

TEST_CASE("_mbsstr's validation order is the reference's")
{
  // An empty needle answers with the haystack before the haystack is
  // checked, so `_mbsstr(nullptr, "")` is nullptr with errno untouched --
  // null in, null out, but not an error.
  errno = ERANGE;
  REQUIRE(_mbsstr(kAbc, (const unsigned char *)"") == kAbc);
  REQUIRE(errno == ERANGE);
  errno = ERANGE;
  REQUIRE(_mbsstr(nullptr, (const unsigned char *)"") == nullptr);
  REQUIRE(errno == ERANGE);

  errno = 0;
  REQUIRE(_mbsstr(kAbc, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsstr(nullptr, (const unsigned char *)"x") == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("the four scan faces are complements of each other")
{
  // _mbsspn asks "how long is the prefix of characters the set contains",
  // _mbscspn "how long is the prefix of characters the set does not", and
  // _mbsspnp / _mbspbrk are the same two questions asked for a pointer.
  // The set below is "b" and U+3042, and kScan is a b U+3042 c d.
  const unsigned char *set = (const unsigned char *)"b\xE3\x81\x82";
  REQUIRE(_mbsspn(kScan, set) == 0);  // 'a' is not in the set
  REQUIRE(_mbscspn(kScan, set) == 1); // 'b' is
  REQUIRE(_mbsspnp(kScan, set) == kScan);
  REQUIRE(_mbspbrk(kScan, set) == kScan + 1);

  const unsigned char *head = (const unsigned char *)"ab\xE3\x81\x82";
  REQUIRE(_mbsspn(kScan, head) == 5);
  REQUIRE(_mbscspn(kScan, head) == 0);
  REQUIRE(_mbsspnp(kScan, head) == kScan + 5);
  REQUIRE(_mbspbrk(kScan, head) == kScan);

  const unsigned char *tail = (const unsigned char *)"cd";
  REQUIRE(_mbsspn(kScan, tail) == 0);
  REQUIRE(_mbscspn(kScan, tail) == 5);
  REQUIRE(_mbsspnp(kScan, tail) == kScan);
  REQUIRE(_mbspbrk(kScan, tail) == kScan + 5);

  // Every character in the set: the spn side runs the whole string, the
  // cspn side stops at the first byte.  kScan is its own complete set.
  const unsigned char *all = kScan;
  REQUIRE(_mbsspn(kScan, all) == kScanLen);
  REQUIRE(_mbscspn(kScan, all) == 0);
  REQUIRE(_mbsspnp(kScan, all) == nullptr);
  REQUIRE(_mbspbrk(kScan, all) == kScan);

  // An empty set contains nothing, so the complement is the whole string.
  const unsigned char *empty = (const unsigned char *)"";
  REQUIRE(_mbsspn(kScan, empty) == 0);
  REQUIRE(_mbscspn(kScan, empty) == kScanLen);
  REQUIRE(_mbsspnp(kScan, empty) == kScan);
  REQUIRE(_mbspbrk(kScan, empty) == nullptr);
}

TEST_CASE("an incomplete set entry matches nothing")
{
  // plan-3 D9n: the reference's dud DBCS pair -- a lead byte followed by
  // the terminator inside the *set* -- is written as a branch that matches
  // any character at all, because there is no sensible way to compare a
  // half character.  Carrying that over would make a typo'd delimiter set
  // swallow the whole string, so the entry is skipped instead.  Here it
  // changes the answer from 0 to 6, which is the difference between "the
  // first character is at offset 0" and "nothing matched at all".
  const unsigned char *dud = (const unsigned char *)"\xE3";
  REQUIRE(_mbscspn(kScan, dud) == kScanLen);
  REQUIRE(_mbsspn(kScan, dud) == 0);
  REQUIRE(_mbspbrk(kScan, dud) == nullptr);
  REQUIRE(_mbsspnp(kScan, dud) == kScan);

  // A stray continuation byte in the set is skipped the same way, having
  // decoded to nothing.
  const unsigned char *stray = (const unsigned char *)"\x81";
  REQUIRE(_mbscspn(kScan, stray) == kScanLen);
  REQUIRE(_mbscspn(kScan, (const unsigned char *)"b\x81") == 1);
}

TEST_CASE("the scan faces validate both pointers")
{
  errno = 0;
  REQUIRE(_mbsspn(nullptr, kAbc) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsspn(kAbc, nullptr) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbscspn(nullptr, kAbc) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbscspn(kAbc, nullptr) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsspnp(nullptr, kAbc) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsspnp(kAbc, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbspbrk(nullptr, kAbc) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbspbrk(kAbc, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("every locate face ignores a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbschr_l(kAHZ, 0x3042, anywhere) == _mbschr(kAHZ, 0x3042));
  REQUIRE(_mbsrchr_l(kAHZ2, 0x3042, anywhere) == _mbsrchr(kAHZ2, 0x3042));
  REQUIRE(_mbsstr_l(kHira2x, kHira, anywhere) == _mbsstr(kHira2x, kHira));
  REQUIRE(_mbsspn_l(kScan, kAbc, anywhere) == _mbsspn(kScan, kAbc));
  REQUIRE(_mbscspn_l(kScan, kAbc, anywhere) == _mbscspn(kScan, kAbc));
  REQUIRE(_mbsspnp_l(kScan, kAbc, anywhere) == _mbsspnp(kScan, kAbc));
  REQUIRE(_mbspbrk_l(kScan, kAbc, anywhere) == _mbspbrk(kScan, kAbc));
}
