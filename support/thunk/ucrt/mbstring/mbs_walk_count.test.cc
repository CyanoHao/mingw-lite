#include <catch_amalgamated.hpp>

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mbs_str_decls.h"

// plan-3 §5.3 line b, first third: the walking and counting faces.  The
// question every assertion here answers is the same one -- "what is a
// character, and where does it begin" -- and each face is pinned against
// the mixed string so a disagreement between two of them is visible.

extern "C"
{
  int __cdecl __ms__setmbcp(int code_page);
} // extern "C"

namespace
{
  // Every non-ASCII fixture below is written as bytes, never as a string
  // literal with hex escapes.  A C hex escape is greedy over the
  // characters that follow it, so "a\xE3\x81\x82z" happens to be safe only
  // because 'z' is not a hex digit -- the next literal to end in a real
  // letter would not be, and the compiler says so only with a warning.

  // 'a' + U+3042 (HIRAGANA LETTER A, 3 bytes) + U+1F600 (GRINNING FACE,
  // 4 bytes) + 'z': one single-byte, one three-byte and one four-byte
  // character, so every width in the engine is on show in nine bytes.
  const unsigned char kMixed[] = {
      'a', 0xE3, 0x81, 0x82, 0xF0, 0x9F, 0x98, 0x80, 'z', 0};
  constexpr size_t kMixedLen = 9;   // bytes, terminator excluded
  constexpr size_t kMixedChars = 4; // characters
  constexpr size_t kZ = 8;          // offset of the trailing 'z'
  constexpr size_t kEnd = 9;        // offset of the terminator

  // The four character starts.
  const size_t kStart[kMixedChars] = {0, 1, 4, 8};

  // A lead byte with the terminator following it: the reference's
  // "invalid MBC", the DBCS shape of an incomplete UTF-8 sequence.
  const unsigned char kTrunc[] = {'a', 0xE3, 0};
  // A complete U+3042 followed by a truncated one.
  const unsigned char kTruncTail[] = {0xE3, 0x81, 0x82, 0xF0, 0x9F, 0x98, 0};
  // A lone continuation byte, which no lead ever claimed.
  const unsigned char kLoneTrail[] = {'a', 0x81, 'z', 0};
} // namespace

// ---------------------------------------------------------------- walking

TEST_CASE("_mbsinc advances by the lead byte's declared width")
{
  REQUIRE(_mbsinc(kMixed) == kMixed + 1);
  REQUIRE(_mbsinc(kMixed + 1) == kMixed + 4);
  REQUIRE(_mbsinc(kMixed + 4) == kMixed + 8);
  REQUIRE(_mbsinc(kMixed + kZ) == kMixed + kEnd); // the terminator

  // plan-3 D9a: the reference adds a flat 2 for a lead byte, because a
  // DBCS pair is two bytes.  Under UTF-8 the declared width is the
  // answer, so a three-byte character moves three and a four-byte one
  // moves four.  This is the one place where the two codepages cannot be
  // made to agree, and the engine is on the UTF-8 side of the line.
  REQUIRE((_mbsinc(kMixed + 1) - (kMixed + 1)) == 3);
  REQUIRE((_mbsinc(kMixed + 4) - (kMixed + 4)) == 4);
}

TEST_CASE("_mbsinc on an incomplete tail lands on the terminator")
{
  // The reference's DBCS form advances once, finds the terminator, and
  // does not take its second step.  The UTF-8 form steps to the end of the
  // string, which for a lead byte immediately before the terminator is the
  // same place and for a longer sequence is the only place a caller could
  // want: the answer is never a pointer into the middle of a character that
  // is not one.
  REQUIRE(_mbsinc(kTrunc) == kTrunc + 1);
  REQUIRE(_mbsinc(kTrunc + 1) == kTrunc + 2);

  // A four-byte lead with three bytes present steps to the terminator, not
  // to the byte after the lead and not four bytes forward past the end.
  REQUIRE(_mbsinc(kTruncTail) == kTruncTail + 3);
  REQUIRE(_mbsinc(kTruncTail + 3) == kTruncTail + 6);

  // A lone continuation byte is not a character start, so it advances one.
  REQUIRE(_mbsinc(kLoneTrail) == kLoneTrail + 1);
  REQUIRE(_mbsinc(kLoneTrail + 1) == kLoneTrail + 2);

  // A lead byte followed by something that is not a trail byte is a dud of
  // its own: it advances one byte and leaves the rest of the string alone,
  // which is the difference between "the terminator cut this character
  // short" and "these bytes were never a character".
  const unsigned char mixed_up[] = {0xE3, 'a', 'b', 0};
  REQUIRE(_mbsinc(mixed_up) == mixed_up + 1);
  REQUIRE(_mbsinc(mixed_up + 1) == mixed_up + 2);
}

TEST_CASE("_mbsinc(NULL) is EINVAL, where wine crashes")
{
  // wine's SBCS shortcut dereferences its argument; the reference's MBCS
  // path dereferences it too.  A null deref is not a behaviour worth
  // reproducing, so the engine answers the reference's own error code.
  errno = 0;
  REQUIRE(_mbsinc(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_mbsinc_l(nullptr, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsdec is the inverse of _mbsinc")
{
  for (size_t k = 1; k < kMixedChars; ++k) {
    INFO("character " << k);
    REQUIRE(_mbsdec(kMixed, kMixed + kStart[k]) == kMixed + kStart[k - 1]);
  }

  // ... and the round trip the plan asks for, the other way round: stepping
  // forward and then back has to land where it started.  (_mbsinc of the
  // answer to _mbsdec is *not* the identity -- _mbsdec hands back the start
  // of the previous character, and moving forward from there lands on the
  // character the caller was inside, which is one position further on.)
  for (size_t k = 0; k + 1 < kMixedChars; ++k) {
    INFO("character " << k);
    REQUIRE(_mbsdec(kMixed, _mbsinc(kMixed + kStart[k])) == kMixed + kStart[k]);
  }

  // Across an incomplete tail the walk is still total: _mbsdec lands on
  // the character before the one the tail sits in.
  REQUIRE(_mbsdec(kTrunc, kTrunc + 1) == kTrunc);
  REQUIRE(_mbsdec(kTruncTail, kTruncTail + 3) == kTruncTail);
}

TEST_CASE("_mbsdec needs a character to step back over")
{
  // The reference returns nullptr for `string >= current` without going
  // through any validation macro, so errno is whatever the caller left.
  // "At or before the start" is about `current` against `string`, not about
  // whether a character exists behind it: one byte in there is still one
  // character to step back over, which is the case below.
  errno = ERANGE;
  REQUIRE(_mbsdec(kMixed, kMixed) == nullptr);
  REQUIRE(errno == ERANGE);
  REQUIRE(_mbsdec(kMixed, kMixed - 1) == nullptr);
  REQUIRE(errno == ERANGE);

  // One byte in is still a move: `current - 1` is the first character, so
  // the answer is the start of the string.  The reference's arithmetic says
  // the same thing -- `current - 1 - ((current - temp) & 1)` with temp
  // walked back below `string` answers index 0 -- and it is the case a
  // "there is no earlier character, so null" reading gets wrong.
  errno = ERANGE;
  REQUIRE(_mbsdec(kMixed, kMixed + 1) == kMixed);
  REQUIRE(errno == ERANGE);

  // One byte in is still a move: `current - 1` is the first character, so
  // the answer is the start of the string.  The reference's arithmetic says
  // the same thing -- `current - 1 - ((current - temp) & 1)` with temp
  // walked back below `string` answers index 0 -- and it is the case a
  // "there is no earlier character, so null" reading gets wrong.
  errno = ERANGE;
  REQUIRE(_mbsdec(kMixed, kMixed + 1) == kMixed);
  REQUIRE(errno == ERANGE);

  // A null on either side is EINVAL.
  errno = 0;
  REQUIRE(_mbsdec(nullptr, kMixed) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsdec(kMixed, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsnextc decodes the whole scalar value")
{
  // plan-3 D9c: the reference returns a 16-bit DBCS code, so `unsigned
  // int` was already wide enough for a UTF-8 scalar and nothing about the
  // signature has to change.
  REQUIRE(_mbsnextc(kMixed) == 0x61);
  REQUIRE(_mbsnextc(kMixed + 1) == 0x3042);
  REQUIRE(_mbsnextc(kMixed + 4) == 0x1F600);
  REQUIRE(_mbsnextc(kMixed + kZ) == 0x7A);

  // A malformed or incomplete character has no scalar value, so 0 -- the
  // same value the reference gives a naked lead byte at EOS.
  REQUIRE(_mbsnextc(kTrunc + 1) == 0);
  REQUIRE(_mbsnextc(kTruncTail + 3) == 0);
  REQUIRE(_mbsnextc(kLoneTrail + 1) == 0);

  errno = 0;
  REQUIRE(_mbsnextc(nullptr) == 0);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsninc advances whole characters")
{
  // The reference reaches this answer by delegating to _mbsnbcnt, so
  // count counts characters and not bytes -- the "n" half of the split.
  REQUIRE(_mbsninc(kMixed, 0) == kMixed);
  REQUIRE(_mbsninc(kMixed, 1) == kMixed + 1);
  REQUIRE(_mbsninc(kMixed, 2) == kMixed + 4);
  REQUIRE(_mbsninc(kMixed, 3) == kMixed + 8);
  REQUIRE(_mbsninc(kMixed, 4) == kMixed + kEnd);

  // Past the end the walk stops at the terminator rather than running off
  // it: 9 is the byte length, 99 is absurd, and both are the terminator.
  REQUIRE(_mbsninc(kMixed, kMixedLen) == kMixed + kEnd);
  REQUIRE(_mbsninc(kMixed, 99) == kMixed + kEnd);

  REQUIRE(_mbsninc(nullptr, 1) == nullptr);
  REQUIRE(_mbsninc(nullptr, 0) == nullptr);
}

// --------------------------------------------------------------- counting

TEST_CASE("_mbsnccnt counts characters, _mbsnbcnt counts bytes")
{
  // The two faces answer different questions about the same string, and
  // the naming does not tell you which is which: the reference's
  // _mbsnccnt takes a *byte* count and _mbsnbcnt takes a *character*
  // count, while _mbsncpy takes characters and _mbsnbcpy takes bytes.
  // Each is anchored here against the reference shell it came from.
  REQUIRE(_mbsnccnt(kMixed, kMixedLen) == kMixedChars);
  REQUIRE(_mbsnbcnt(kMixed, kMixedChars) == kMixedLen);

  // nccnt: a character the byte bound would split is not counted, and
  // ends the walk (the reference's `!bcnt--` break).
  REQUIRE(_mbsnccnt(kMixed, 0) == 0);
  REQUIRE(_mbsnccnt(kMixed, 1) == 1); // 'a'
  REQUIRE(_mbsnccnt(kMixed, 2) == 1); // U+3042 does not fit in 2
  REQUIRE(_mbsnccnt(kMixed, 3) == 1);
  REQUIRE(_mbsnccnt(kMixed, 4) == 2); // 'a' + U+3042
  REQUIRE(_mbsnccnt(kMixed, 5) == 2);
  REQUIRE(_mbsnccnt(kMixed, 7) == 2);
  REQUIRE(_mbsnccnt(kMixed, 8) == 3); // + U+1F600 exactly

  // nbcnt: the same numbers the other way round.
  REQUIRE(_mbsnbcnt(kMixed, 0) == 0);
  REQUIRE(_mbsnbcnt(kMixed, 1) == 1);
  REQUIRE(_mbsnbcnt(kMixed, 2) == 4);
  REQUIRE(_mbsnbcnt(kMixed, 3) == 8);
  REQUIRE(_mbsnbcnt(kMixed, 4) == 9);
  REQUIRE(_mbsnbcnt(kMixed, 99) == 9);
}

TEST_CASE("nccnt and nbcnt agree on where every character ends")
{
  // The two faces are inverses over a well-formed prefix, which is the
  // strongest statement available about the "never half a sequence" rule:
  // for every byte bound b, the bytes the character count names are a
  // character boundary.
  for (size_t b = 0; b <= kMixedLen; ++b) {
    INFO("byte bound " << b);
    const size_t chars = _mbsnccnt(kMixed, b);
    REQUIRE(_mbsnbcnt(kMixed, chars) <= b);
  }
}

TEST_CASE("nccnt and nbcnt stop before an incomplete tail")
{
  // The reference leaves a dangling lead byte out of both totals: nccnt
  // breaks out of its loop and nbcnt does `--p; break`.  Counting it
  // would make the two disagree about the string's length, and a caller
  // using both to walk one string would step past its own terminator.
  REQUIRE(_mbsnccnt(kTrunc, 2) == 1);
  REQUIRE(_mbsnbcnt(kTrunc, 2) == 1);
  REQUIRE(_mbsnccnt(kTruncTail, 6) == 1);
  REQUIRE(_mbsnbcnt(kTruncTail, 2) == 3);

  // A byte that no lead ever claimed is a character of its own, which is
  // the reference's counting rule: only a *lead* means "a character was
  // started and never finished".  A stricter engine that refused to
  // decode it would report 1 here and disagree with _mbslen below.
  REQUIRE(_mbsnccnt(kLoneTrail, 3) == 3);
  REQUIRE(_mbsnbcnt(kLoneTrail, 2) == 2);
  REQUIRE(_mbsnbcnt(kLoneTrail, 3) == 3);

  errno = 0;
  REQUIRE(_mbsnccnt(nullptr, 1) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnbcnt(nullptr, 1) == 0);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbslen counts characters and _mbsnlen bounds the count")
{
  // The reference shell's own comment settles the unit: "Find the length
  // of the MBCS string (in characters)".
  REQUIRE(_mbslen(kMixed) == 4);
  REQUIRE(_mbslen((const unsigned char *)"") == 0);
  REQUIRE(_mbslen((const unsigned char *)"abc") == 3);
  REQUIRE(_mbslen((const unsigned char *)"\xE3\x81\x82") == 1);

  // A truncated tail is not a character, so it is not counted.
  REQUIRE(_mbslen(kTrunc) == 1);
  REQUIRE(_mbslen(kTruncTail) == 1);

  REQUIRE(_mbsnlen(kMixed, 0) == 0);
  REQUIRE(_mbsnlen(kMixed, 1) == 1);
  REQUIRE(_mbsnlen(kMixed, 2) == 2);
  REQUIRE(_mbsnlen(kMixed, 4) == 4);
  REQUIRE(_mbsnlen(kMixed, 99) == 4);

  // The reference has no validation section for either face, so a null
  // string would deref.  Hardened here, and the difference is recorded.
  errno = 0;
  REQUIRE(_mbslen(nullptr) == 0);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnlen(nullptr, 4) == 0);
  REQUIRE(errno == EINVAL);
}

// ------------------------------------------------- _mbstrlen / _mbstrnlen

TEST_CASE("_mbstrlen counts characters and reports EILSEQ")
{
  // Same number as _mbslen on a well-formed string, and a different
  // answer on a broken one: this face validates, the older one does not.
  REQUIRE(_mbstrlen((const char *)kMixed) == 4);
  REQUIRE(_mbstrlen("") == 0);
  REQUIRE(_mbstrlen("abc") == 3);

  // A sequence the terminator cuts short: the reference's
  // MultiByteToWideChar(MB_ERR_INVALID_CHARS) precheck rejects the whole
  // string, so there is no partial answer to report.
  errno = 0;
  REQUIRE(_mbstrlen((const char *)kTrunc) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrlen((const char *)kTruncTail) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrlen((const char *)"\x81") == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // The reference's own header says the argument is not validated here, so
  // a null would reach strlen.  Hardened to the EINVAL the _mbstrnlen
  // twin reports for the same pointer.
  errno = 0;
  REQUIRE(_mbstrlen(nullptr) == (size_t)-1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbstrnlen's bound is bytes, and the window is validated")
{
  // The reference's return is `size >= max_size ? max_size : n`, so the
  // character count is what a caller gets only when max_size is *larger*
  // than the string.  Hand the exact byte length and you get the byte
  // length back, which is the "safe buffer" idiom working: the point of
  // passing strlen + 1 is precisely to be able to read the answer.
  REQUIRE(_mbstrnlen((const char *)kMixed, 10) == 4);
  REQUIRE(_mbstrnlen((const char *)kMixed, 99) == 4);

  // A bound that reaches the string reports the bound.  Four bytes is
  // exactly 'a' and U+3042, so that window is well formed and holds no
  // terminator: the bound comes back.
  REQUIRE(_mbstrnlen((const char *)kMixed, 9) == 9);
  REQUIRE(_mbstrnlen((const char *)kMixed, 8) == 8);
  REQUIRE(_mbstrnlen((const char *)kMixed, 4) == 4);
  REQUIRE(_mbstrnlen((const char *)kMixed, 1) == 1);

  // A bound that lands inside a character is EILSEQ, not a truncated
  // count.  That is the difference between this face and _mbsnlen, whose
  // count is characters from the start and never inspects the tail.  Five
  // through seven all stop inside U+1F600, so the window is checked and
  // rejected for each of them -- the bound is not silently rounded down to
  // a whole number of characters.
  for (size_t bound = 5; bound <= 7; ++bound) {
    INFO("bound " << bound);
    errno = 0;
    REQUIRE(_mbstrnlen((const char *)kMixed, bound) == (size_t)-1);
    REQUIRE(errno == EILSEQ);
  }
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kMixed, 2) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kMixed, 3) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kMixed, 7) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // A bound that stops short of a cut tail never sees the tail, so the
  // same string is fine at one bound and EILSEQ at every larger one.  The
  // 3-byte window holds U+3042 exactly, so the answer is the bound.
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kTruncTail, 3) == 3);
  REQUIRE(errno == 0);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kTruncTail, 4) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kTruncTail, 6) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kTruncTail, 7) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // The two argument checks sit in the face, exactly where the reference
  // puts them in _mbstrnlen_l rather than in the shared body.
  errno = 0;
  REQUIRE(_mbstrnlen(nullptr, 0) == (size_t)-1);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kMixed, (size_t)INT_MAX + 1) == (size_t)-1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsnlen and _mbstrnlen disagree in three separate ways")
{
  // Same string, same numeric bound, different questions -- which is
  // exactly why ucrt has both faces and why collapsing them would be a
  // silent behaviour change.
  //
  //   bound 2:  _mbsnlen says 2 characters, _mbstrnlen says EILSEQ
  //             because the window cuts U+3042 in half.
  //   bound 8:  _mbsnlen says 4 characters (its count is characters from
  //             the start, so 8 characters is simply the whole string),
  //             _mbstrnlen says 8 because the byte tally reached the
  //             bound and the byte tally is the answer in that case.
  //   cut tail: _mbsnlen counts what is there, _mbstrnlen rejects it.
  REQUIRE(_mbsnlen(kMixed, 2) == 2);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kMixed, 2) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  REQUIRE(_mbsnlen(kMixed, 8) == 4);
  REQUIRE(_mbstrnlen((const char *)kMixed, 8) == 8);

  REQUIRE(_mbsnlen(kTruncTail, 99) == 1);
  errno = 0;
  REQUIRE(_mbstrnlen((const char *)kTruncTail, 99) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // The one case where they agree, so the disagreements above cannot be
  // explained by one face being simply broken.
  REQUIRE(_mbsnlen(kMixed, 4) == 4);
  REQUIRE(_mbstrnlen((const char *)kMixed, 4) == 4);
}

TEST_CASE("the walking faces ignore a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;
  REQUIRE(_mbsinc_l(kMixed, anywhere) == _mbsinc(kMixed));
  // _mbsdec moves back over the character *containing* current - 1, which
  // makes a pointer into the middle of a character answer that character's
  // own start, and a pointer at a character's first byte answer the start
  // of the one before it.  kMixed's starts are 0, 1, 4 and 8, so each of
  // these is one step behind where the helper's name might suggest.
  REQUIRE(_mbsdec_l(kMixed, kMixed + 1, anywhere) == kMixed);
  REQUIRE(_mbsdec_l(kMixed, kMixed + 4, anywhere) == kMixed + 1);
  REQUIRE(_mbsdec_l(kMixed, kMixed + 5, anywhere) == kMixed + 4);
  REQUIRE(_mbsdec_l(kMixed, kMixed + 8, anywhere) == kMixed + 4);
  REQUIRE(_mbsnextc_l(kMixed + 1, anywhere) == 0x3042);
  REQUIRE(_mbsninc_l(kMixed, 2, anywhere) == kMixed + 4);
  REQUIRE(_mbsnccnt_l(kMixed, 4, anywhere) == 2);
  REQUIRE(_mbsnbcnt_l(kMixed, 2, anywhere) == 4);
  REQUIRE(_mbslen_l(kMixed, anywhere) == 4);
  REQUIRE(_mbsnlen_l(kMixed, 2, anywhere) == 2);
  REQUIRE(_mbstrlen_l((const char *)kMixed, anywhere) == 4);
  // A bound the string fills exactly answers the *bound*, not the character
  // count: the reference's `size >= max_size ? max_size : n` only has a
  // count to give when the terminator is inside the window, which ten bytes
  // of a nine-byte string provides and nine does not.
  REQUIRE(_mbstrnlen_l((const char *)kMixed, 9, anywhere) == 9);
  REQUIRE(_mbstrnlen_l((const char *)kMixed, 10, anywhere) == 4);
}

TEST_CASE("M13b walking faces pollution adversarial")
{
  // plan-3 §5.3 pollution case / D9: asking native to enter a DBCS code
  // page must not reach this engine.  M10's _setmbcp refuses every page
  // but UTF-8, so the call below succeeds and changes nothing -- and if
  // any of these answers moves, the engine is reading native state.
  REQUIRE(__ms__setmbcp(936) == 0);
  REQUIRE(_mbsinc(kMixed) == kMixed + 1);
  REQUIRE(_mbsinc(kMixed + 1) == kMixed + 4);
  REQUIRE(_mbsdec(kMixed, kMixed + 8) == kMixed + 4);
  REQUIRE(_mbsdec(kMixed, kMixed + 5) == kMixed + 4);
  REQUIRE(_mbsnextc(kMixed + 4) == 0x1F600);
  REQUIRE(_mbsnccnt(kMixed, 8) == 3);
  REQUIRE(_mbsnbcnt(kMixed, 3) == 8);
  REQUIRE(_mbslen(kMixed) == 4);
  REQUIRE(_mbstrlen((const char *)kMixed) == 4);
  REQUIRE(_mbstrnlen((const char *)kMixed, 9) == 9);
  __ms__setmbcp(65001);
}
