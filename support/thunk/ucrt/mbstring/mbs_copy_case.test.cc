#include <catch_amalgamated.hpp>

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mbs_str_decls.h"

// plan-3 §5.3 line b, last third: the faces that write.  Copy, cat, set,
// case, reverse, tokenize and duplicate -- thirty-odd faces over one
// engine, and the mistakes they invite are all of the same shape: a bound
// in the wrong unit, a character split at a boundary, a buffer reset on
// the wrong side of a return value.  Every test below is one of those
// three.

namespace
{
  // Every non-ASCII fixture below is written as bytes, never as a string
  // literal with hex escapes.  A C hex escape is greedy over the
  // characters that follow it, so "\xE3\x81\x82a" is not "a three-byte
  // character then a" -- it is one out-of-range escape, and the compiler
  // says so only with a warning.

  // 'a' + U+3042 + U+1F600 + 'z', nine bytes and four characters.
  const unsigned char kMixed[] = {
      'a', 0xE3, 0x81, 0x82, 0xF0, 0x9F, 0x98, 0x80, 'z', 0};
  constexpr size_t kMixedLen = 9;

  const unsigned char kHira[] = {0xE3, 0x81, 0x82, 0}; // U+3042
  const unsigned char kHira2[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 0};
  const unsigned char kHira2x[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 'x', 0};

  // One accented letter's worth of case pair, in UTF-8: U+00C0 A-grave and
  // U+00C9 E-acute above, U+00E0 and U+00E9 below.  Two bytes per
  // character, and the two sides are the same length, so a fold that moves
  // either of them is an exact in-place rewrite.  Two *different* letters,
  // because a pair like (U+00C0, U+00E0) would leave the second character
  // already folded and a fold that only ever moves half its input is not
  // much of a test.  (The Latin-1 *bytes* C0/E9 are not decodable UTF-8 at
  // all -- C0 as a lead byte can only begin an overlong form -- and this
  // layer reports them as EILSEQ, which is its own case below.)
  const unsigned char kUpperLatin[] = {0xC3, 0x80, 0xC3, 0x89, 0};
  const unsigned char kLowerLatin[] = {0xC3, 0xA0, 0xC3, 0xA9, 0};

  // 'a' + U+3042 + 'z' and the same with a second U+3042.
  const unsigned char kAHZ[] = {'a', 0xE3, 0x81, 0x82, 'z', 0};
  const unsigned char kAHZ2[] = {
      'a', 0xE3, 0x81, 0x82, 'z', 0xE3, 0x81, 0x82, 0};

  // A lead byte the terminator cuts short.
  const unsigned char kCut[] = {'a', 0xE3, 0};

  // The window the `_s` case matrix folds in.  It has to be at least as
  // large as the largest `size` that matrix passes, because `_FILL_STRING`
  // and `_RESET_STRING` both work on the whole window: the secure faces
  // write `size` bytes even when the string in them is three bytes long.
  constexpr size_t kWin = 128;
  const unsigned char kZeroes[kWin] = {};

  // A destination big enough that the test never has to think about the
  // tail, plus a size exactly one more than the content so the terminator
  // fits and nothing else does.
  constexpr size_t kBig = 32;
} // namespace

// --------------------------------------------------------------- dup

TEST_CASE("_mbsdup")
{
  // wine anchors: a null string answers null and leaves errno alone, and
  // an empty string still allocates so the caller can free unconditionally.
  errno = ERANGE;
  REQUIRE(_mbsdup(nullptr) == nullptr);
  REQUIRE(errno == ERANGE);

  unsigned char *empty = _mbsdup((const unsigned char *)"");
  REQUIRE(empty != nullptr);
  REQUIRE(*empty == '\0');
  free(empty);

  unsigned char *copy = _mbsdup(kMixed);
  REQUIRE(copy != nullptr);
  REQUIRE(copy != kMixed);
  REQUIRE(memcmp(copy, kMixed, kMixedLen + 1) == 0);
  free(copy);
}

// ------------------------------------------------------ case: the _s matrix

TEST_CASE("_mbslwr_s and _mbsupr_s follow the wine matrix")
{
  // Every row below was read off wine, not off the documentation, and
  // they are the reason this face exists in the layer: the four "both
  // arguments wrong" shapes give four different answers.  The two faces
  // share the engine body, so the matrix runs against both -- a fork here
  // would be a fork nobody asked for.
  struct Case
  {
    const char *input;
    size_t size;
    errno_t want;
    // What the buffer holds afterwards, per face; nullptr means "the buffer
    // must be untouched".  Two columns rather than one because the two
    // faces fold in opposite directions, and a shared column would only ever
    // be right for one of them.
    const char *lwr;
    const char *upr;
  };
  const Case matrix[] = {
      // the string is terminated inside size: fold it
      {"AbC", 4, 0, "abc", "ABC"},
      // and it is still terminated inside a wildly over-large size
      {"AbC", 99, 0, "abc", "ABC"},
      // not terminated inside size: EINVAL, and the buffer is reset
      {"AbC", 3, EINVAL, "", ""},
      // size 0 with a real pointer: EINVAL, and the buffer is *not*
      // touched, which is the row a "reset everything" reading gets wrong
      {"AbC", 0, EINVAL, "AbC", "AbC"},
  };

  for (const Case &c : matrix) {
    {
      INFO("lwr \"" << c.input << "\" size " << c.size);
      // A real three-character string in a buffer whose tail is already
      // zero: the size 3 row is the interesting one, and it only means
      // "unterminated inside size" because the byte at offset 3 is NUL.
      //
      // The buffer is as large as the largest size the matrix passes, and
      // that is not tidiness: the `_s` contract fills or resets the *whole*
      // size window, so a size of 99 means 99 bytes of writes whether or
      // not the string is three characters long.  A buffer sized to the
      // string would be a stack smash on the over-large row, and the
      // "reset" row would only ever reset the three bytes it could reach.
      unsigned char buf[kWin] = {'A', 'b', 'C'};
      REQUIRE(_mbslwr_s(buf, c.size) == c.want);
      if (c.lwr) {
        const size_t n = strlen((const char *)buf);
        REQUIRE(strcmp((const char *)buf, c.lwr) == 0);
        REQUIRE(memcmp(buf + n + 1, kZeroes, kWin - n - 1) == 0);
      } else {
        REQUIRE(memcmp(buf, "AbC", 4) == 0);
      }
    }
    {
      INFO("upr \"" << c.input << "\" size " << c.size);
      unsigned char buf[kWin] = {'A', 'b', 'C'};
      REQUIRE(_mbsupr_s(buf, c.size) == c.want);
      if (c.upr) {
        const size_t n = strlen((const char *)buf);
        REQUIRE(strcmp((const char *)buf, c.upr) == 0);
        REQUIRE(memcmp(buf + n + 1, kZeroes, kWin - n - 1) == 0);
      } else {
        REQUIRE(memcmp(buf, "AbC", 4) == 0);
      }
    }
  }

  // The two degenerate argument pairs, verbatim from the probe.
  REQUIRE(_mbslwr_s(nullptr, 0) == 0);
  REQUIRE(_mbsupr_s(nullptr, 0) == 0);
  errno = 0;
  REQUIRE(_mbslwr_s(nullptr, 4) == EINVAL);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsupr_s(nullptr, 4) == EINVAL);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("the in-place fold reaches the non-ASCII cases the tables have")
{
  // Both accented letters move, and both stay two bytes wide, so the
  // in-place rewrite is exact and the answer is a fixture rather than a
  // recount.
  unsigned char latin[5];
  memcpy(latin, kUpperLatin, 5);
  REQUIRE(_mbslwr(latin) == latin);
  REQUIRE(memcmp(latin, kLowerLatin, 5) == 0);

  unsigned char upper[5];
  memcpy(upper, kLowerLatin, 5);
  REQUIRE(_mbsupr(upper) == upper);
  REQUIRE(memcmp(upper, kUpperLatin, 5) == 0);

  // The Latin-1 *bytes* C0 and E9 are not decodable UTF-8 -- C0 as a lead
  // byte can only begin an overlong form, which this layer rejects -- so a
  // string built from them is the malformed case, not the case case.  It
  // is worth its own test because it is the shape a caller porting Latin-1
  // data reaches first.
  unsigned char raw[3] = {0xC0, 0xE9, 0};
  errno = 0;
  REQUIRE(_mbslwr(raw) == nullptr);
  REQUIRE(errno == EILSEQ);
  // The fold truncated the string at the first byte that starts no
  // character, so the buffer cannot be reused for the next face -- there is
  // nothing left in it to fail.  A fresh copy of the same bytes is what
  // _mbstrlen sees, and counting characters means decoding them, which is
  // where this layer parts company with the byte-counting faces above.
  REQUIRE(raw[0] == 0);
  const unsigned char raw2[3] = {0xC0, 0xE9, 0};
  REQUIRE(_mbstrlen((const char *)raw2) == (size_t)-1);

  // Kana does not fold in the invariant table this layer reaches through,
  // so a string of it comes back byte for byte and with no complaint.
  unsigned char kana[4] = {0xE3, 0x81, 0x82, 0};
  REQUIRE(_mbslwr(kana) == kana);
  REQUIRE(memcmp(kana, kHira, 4) == 0);

  // A string mixing a foldable character with characters that do not move
  // is folded in place, character by character, and the unmoved ones are
  // left exactly as they were.
  unsigned char mixed[6] = {'A', 0xE3, 0x81, 0x82, 'B', 0};
  REQUIRE(_mbslwr(mixed) == mixed);
  REQUIRE(mixed[0] == 'a');
  REQUIRE(memcmp(mixed + 1, kHira, 3) == 0);
  REQUIRE(mixed[4] == 'b');
}

TEST_CASE("a character the terminator cuts short fails the in-place fold")
{
  // fold_in_place refuses a byte that does not start a character, which is
  // the reference's naked-lead-byte case, and reports it as its
  // LCMapStringA failure: EILSEQ, and the characters folded before the
  // dud are kept.  The reference's own `_mbslwr` spells the whole face as
  // `_mbslwr_s_l(s, (size_t)-1) == 0 ? s : nullptr`, and this is what that
  // unbounded size has to do -- there is no buffer to reset, because the
  // size does not name one.
  unsigned char cut[3] = {'A', 0xE3, 0};
  errno = 0;
  REQUIRE(_mbslwr(cut) == nullptr);
  REQUIRE(errno == EILSEQ);
  // The characters folded before the dud are kept and the dud is dropped:
  // the result is a decodable one-character string, not the input with an
  // error code attached.
  REQUIRE(cut[0] == 'a');
  REQUIRE(cut[1] == 0);
  REQUIRE(_mbstrlen((const char *)cut) == 1);
}

// ------------------------------------------------------------ reverse

TEST_CASE("_mbsrev reverses characters where _strrev reverses bytes")
{
  unsigned char bychar[16] = {
      'a', 0xE3, 0x81, 0x82, 'z', 0xE3, 0x81, 0x82, 0, 0, 0, 0, 0, 0, 0, 0};
  REQUIRE(_mbsrev(bychar) == bychar);
  // U+3042, 'z', U+3042, 'a'
  const unsigned char kRev[] = {
      0xE3, 0x81, 0x82, 'z', 0xE3, 0x81, 0x82, 'a', 0};
  REQUIRE(memcmp(bychar, kRev, 9) == 0);

  // The byte-reversing face on the same bytes produces mojibake: a lead
  // byte is now at the end of a character, and the string no longer
  // decodes.  Asserting that is the point -- the two faces are not
  // interchangeable and this is what the difference costs.
  unsigned char bybyte[16] = {
      'a', 0xE3, 0x81, 0x82, 'z', 0xE3, 0x81, 0x82, 0, 0, 0, 0, 0, 0, 0, 0};
  REQUIRE(_strrev((char *)bybyte) == (char *)bybyte);
  REQUIRE(memcmp(bybyte, bychar, 9) != 0);
  // 0x82 is a continuation byte in front of nothing.
  REQUIRE(bybyte[0] == 0x82);
  REQUIRE(_mbstrlen((const char *)bybyte) == (size_t)-1);

  // Reversal is its own inverse for a well-formed string, which is the
  // cheapest way to show the character alignment held all the way through.
  REQUIRE(_mbsrev(bychar) == bychar);
  REQUIRE(memcmp(bychar, kAHZ2, 9) == 0);

  unsigned char single[8] = {'a', 0, 0, 0, 0, 0, 0, 0};
  REQUIRE(_mbsrev(single) == single);
  REQUIRE(memcmp(single, "a", 2) == 0);
  unsigned char none[2] = {0, 0};
  REQUIRE(_mbsrev(none) == none);

  errno = 0;
  REQUIRE(_mbsrev(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsrev truncates a string whose last character is incomplete")
{
  // Reversing would attach the orphan lead byte to the character in front
  // of it, and there is no way to know how many bytes to steal.  The
  // reference stops the string at the dud and says so; the engine drops
  // the dud, leaves the complete prefix in place, and reports EINVAL.
  unsigned char cut[8] = {0xE3, 0x81, 0x82, 0xF0, 0x9F, 0x98, 0, 0};
  errno = 0;
  REQUIRE(_mbsrev(cut) == cut);
  REQUIRE(errno == EINVAL);
  REQUIRE(memcmp(cut, kHira, 4) == 0);

  // Same answer from the _s form's engine path, and the string it was
  // handed is still a decodable string afterwards.
  REQUIRE(_mbstrlen((const char *)cut) == 1);
}

// ------------------------------------------------------------ copy / cat

TEST_CASE("_mbsncpy counts characters and _mbsnbcpy counts bytes")
{
  // The same source, the same number, two different answers -- the
  // clearest statement of the family's n / nb split anywhere in the layer.
  // A character count of 2 takes "a" and U+3042 (four bytes); a byte
  // count of 2 can only take "a", because the next character does not fit
  // in what is left and this layer never splits one.
  unsigned char bychars[kBig];
  memset(bychars, 'x', kBig);
  REQUIRE(_mbsncpy(bychars, kMixed, 2) == bychars);
  // memcmp, not strcmp: taking two characters out of a four-character
  // source leaves the destination unterminated, exactly as strncpy does
  // when the source is longer than the count.  The four bytes are there and
  // the terminator is not -- and the two rows below are the ones that add
  // one, which is the whole n / nb difference in a single line.
  REQUIRE(memcmp(bychars, kMixed, 4) == 0);
  REQUIRE(bychars[4] == 'x');

  unsigned char bybytes[kBig];
  memset(bybytes, 'x', kBig);
  REQUIRE(_mbsnbcpy(bybytes, kMixed, 2) == bybytes);
  REQUIRE(strcmp((const char *)bybytes, "a") == 0);

  // A character count outruns its own byte length, so the destination
  // holds more bytes than `count`.  That is the price of the unit, and it
  // is why these faces have no size parameter: the caller owns a buffer
  // three times `count` bytes at worst.  Padding fills the destination
  // out to count *characters*, so an all-ASCII source gives strncpy's
  // answer exactly.
  unsigned char padded[kBig];
  memset(padded, 'x', kBig);
  REQUIRE(_mbsncpy(padded, (const unsigned char *)"ab", 5) == padded);
  REQUIRE(strcmp((const char *)padded, "ab\0\0\0") == 0);

  unsigned char pbytes[kBig];
  memset(pbytes, 'x', kBig);
  REQUIRE(_mbsnbcpy(pbytes, (const unsigned char *)"ab", 5) == pbytes);
  REQUIRE(strcmp((const char *)pbytes, "ab\0\0\0") == 0);
}

TEST_CASE("the n-forms validate nothing on a zero count")
{
  // The reference's `if (!cnt) return dst` sits ahead of its validation
  // section in all four faces, and so does this.  errno is cleared first
  // because the point of the row is that the early return touches *nothing*
  // -- it does not validate, and it does not report either.  Clearing it
  // here is the only way the assertion below says anything at all.
  errno = 0;
  REQUIRE(_mbsncpy(nullptr, kMixed, 0) == nullptr);
  REQUIRE(_mbsnbcpy(nullptr, kMixed, 0) == nullptr);
  REQUIRE(_mbsncat(nullptr, kMixed, 0) == nullptr);
  REQUIRE(_mbsnbcat(nullptr, kMixed, 0) == nullptr);
  REQUIRE(errno == 0);

  errno = 0;
  REQUIRE(_mbsncpy(nullptr, kMixed, 1) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbsnbcpy((unsigned char *)"x", nullptr, 1) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("_mbsncat and _mbsnbcat append, counting characters and bytes")
{
  unsigned char bychars[kBig];
  memset(bychars, 0, kBig);
  REQUIRE(_mbsncat(bychars, kMixed, 2) == bychars);
  REQUIRE(strcmp((const char *)bychars, "a\xE3\x81\x82") == 0);

  unsigned char bybytes[kBig];
  memset(bybytes, 0, kBig);
  REQUIRE(_mbsnbcat(bybytes, kMixed, 2) == bybytes);
  REQUIRE(strcmp((const char *)bybytes, "a") == 0);

  // Appending onto an existing string, with a count of 1: one *character*,
  // which is the whole three bytes of U+3042 and not the whole source.
  unsigned char dst[kBig] = "XY";
  REQUIRE(_mbsncat(dst, kHira2x, 1) == dst);
  const unsigned char want[] = {'X', 'Y', 0xE3, 0x81, 0x82, 0};
  REQUIRE(memcmp(dst, want, 6) == 0);
  // A count past the end of the source takes all of it.
  unsigned char all_of_it[kBig] = "XY";
  REQUIRE(_mbsncat(all_of_it, kHira2x, 99) == all_of_it);
  const unsigned char want_all[] = {
      'X', 'Y', 0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 'x', 0};
  REQUIRE(memcmp(all_of_it, want_all, 10) == 0);

  // A destination ending in an incomplete character drops that character
  // instead of letting the source's first byte complete it into something
  // the caller never wrote.  The reference backs its pointer up one byte
  // over the same test.
  unsigned char glued[kBig] = {'X', 'Y', 0xE3, 0, 0, 0, 0, 0};
  REQUIRE(_mbsncat(glued, (const unsigned char *)"Z", 1) == glued);
  REQUIRE(strcmp((const char *)glued, "XYZ") == 0);
}

TEST_CASE("the n-forms drop a source tail the terminator cut short")
{
  // The cut character is not a character, so the destination holds the
  // prefix that is one.  The reference reaches the same place with
  // `dst[-2] = '\0'`, which happens to name the right offset for a
  // two-byte dud pair and would not for a four-byte one -- the offset is
  // read off the source here instead of computed from the lead byte.
  unsigned char dst[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsncpy(dst, kCut, 8) == dst);
  REQUIRE(strcmp((const char *)dst, "a") == 0);

  unsigned char cpy2[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsnbcpy(cpy2, kCut, 8) == cpy2);
  REQUIRE(strcmp((const char *)cpy2, "a") == 0);
}

// ------------------------------------------------------ the _s protocol

TEST_CASE("_mbscpy_s is the reference's validation order")
{
  unsigned char buf[kBig];
  memset(buf, 'x', kBig);

  REQUIRE(_mbscpy_s(buf, kBig, kMixed) == 0);
  REQUIRE(memcmp(buf, kMixed, kMixedLen) == 0);
  REQUIRE(buf[kMixedLen] == 0);
  for (size_t k = kMixedLen + 1; k < kBig; ++k)
    REQUIRE(buf[k] == 0);

  // A destination of the exact size the source needs: the terminator fits,
  // so this is the boundary case and it succeeds.
  REQUIRE(_mbscpy_s(buf, kMixedLen + 1, kMixed) == 0);

  // One byte short of it: ERANGE, and the destination is reset.  The reset
  // covers `size` bytes and no more -- a caller who passed a nine-byte
  // destination did not hand over the other 23 bytes of the array it
  // happens to be a view into.
  memset(buf, 'x', kBig);
  errno = 0;
  REQUIRE(_mbscpy_s(buf, kMixedLen, kMixed) == ERANGE);
  REQUIRE(errno == ERANGE);
  REQUIRE(buf[kMixedLen - 1] == 0);
  REQUIRE(buf[kMixedLen] == 'x');

  // A null source: EINVAL, also with the buffer reset.  The two failure
  // modes are told apart only by the return value, which is exactly why
  // both reset -- a caller that ignores it must not read a stale string
  // either way.
  memset(buf, 'x', kBig);
  errno = 0;
  REQUIRE(_mbscpy_s(buf, kBig, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[kBig - 1] == 0);

  // A null destination, or a zero size, is EINVAL with nothing to reset.
  errno = 0;
  REQUIRE(_mbscpy_s(nullptr, 0, kMixed) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(_mbscpy_s(buf, 0, kMixed) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(_mbscpy_s(nullptr, kBig, kMixed) == EINVAL);
  REQUIRE(errno == EINVAL);

  // The capacity check runs before any byte is written, so the ERANGE path
  // cannot have overrun the buffer to discover it did not fit.
  unsigned char small[4];
  memset(small, 'x', sizeof small);
  REQUIRE(_mbscpy_s(small, sizeof small, kMixed) == ERANGE);
  REQUIRE(small[0] == 0);
}

TEST_CASE("a counted copy of zero resets the destination and never reads src")
{
  // The reference's `if (srclen == 0)` arm sits ahead of the null check, so
  // a null source with a zero count is success and not EINVAL.  "Do
  // nothing" and "fail" have to be told apart here, and this is the row
  // that tells them apart.
  unsigned char buf[kBig];
  memset(buf, 'x', kBig);
  REQUIRE(_mbsncpy_s(buf, kBig, nullptr, 0) == 0);
  REQUIRE(buf[0] == 0);
  REQUIRE(buf[kBig - 1] == 0);

  memset(buf, 'x', kBig);
  REQUIRE(_mbsncpy_s(buf, kBig, kMixed, 0) == 0);
  REQUIRE(buf[0] == 0);

  errno = 0;
  REQUIRE(_mbsnbcpy_s(buf, kBig, nullptr, 0) == 0);
  REQUIRE(errno == 0);

  // With a live count, a null source is EINVAL again.
  memset(buf, 'x', kBig);
  errno = 0;
  REQUIRE(_mbsncpy_s(buf, kBig, nullptr, 1) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[kBig - 1] == 0);
}

TEST_CASE("the secure n / nb copies keep the same split as the plain ones")
{
  // Same numbers, same source, four different answers, which is the
  // cheapest way to show the secure forms did not quietly collapse the
  // unit the way a single shared helper would have.
  unsigned char bychars[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsncpy_s(bychars, kBig, kMixed, 2) == 0);
  REQUIRE(strcmp((const char *)bychars, "a\xE3\x81\x82") == 0);

  unsigned char bybytes[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsnbcpy_s(bybytes, kBig, kMixed, 2) == 0);
  REQUIRE(strcmp((const char *)bybytes, "a") == 0);

  // Four bytes is exactly one multi-byte character for both, and three is
  // not enough for the byte-counted face.
  unsigned char three[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsnbcpy_s(three, kBig, kMixed, 3) == 0);
  REQUIRE(strcmp((const char *)three, "a") == 0);
  unsigned char four[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsnbcpy_s(four, kBig, kMixed, 4) == 0);
  REQUIRE(strcmp((const char *)four, "a\xE3\x81\x82") == 0);

  // A count past the end of the source copies the source, with the
  // terminator and the zeroed tail the protocol requires -- no padding to
  // `count` characters, which is the one thing the secure form must not do
  // when the destination is a fixed-size buffer.
  unsigned char tail[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  REQUIRE(_mbsncpy_s(tail, kBig, kHira, 99) == 0);
  REQUIRE(strcmp((const char *)tail, "\xE3\x81\x82") == 0);
  REQUIRE(tail[4] == 0);
}

TEST_CASE("_mbscat_s validates the destination before the source")
{
  unsigned char buf[kBig] = "XY";
  REQUIRE(_mbscat_s(buf, kBig, kHira) == 0);
  REQUIRE(strcmp((const char *)buf, "XY\xE3\x81\x82") == 0);
  REQUIRE(buf[5] == 0);
  for (size_t k = 6; k < kBig; ++k)
    REQUIRE(buf[k] == 0);

  // A null source is EINVAL with the buffer reset.
  memset(buf, 'x', kBig);
  errno = 0;
  REQUIRE(_mbscat_s(buf, kBig, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[kBig - 1] == 0);

  // A destination with no terminator inside size is EINVAL with the buffer
  // reset.  The reference cannot look past the buffer to tell whether the
  // string ended in a dud character, so it reports rather than guesses.
  memset(buf, 'x', kBig);
  errno = 0;
  REQUIRE(_mbscat_s(buf, 3, kHira) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[2] == 0);

  // Not enough room for the result is ERANGE, also with the buffer reset.
  unsigned char small[5] = "XY";
  errno = 0;
  REQUIRE(_mbscat_s(small, sizeof small, kHira) == ERANGE);
  REQUIRE(errno == ERANGE);
  REQUIRE(small[0] == 0);
}

TEST_CASE("the secure append keeps the n / nb split too")
{
  unsigned char bychars[kBig] = {'Z', 0, 0, 0, 0, 0, 0, 0};
  REQUIRE(_mbsncat_s(bychars, kBig, kMixed, 2) == 0);
  REQUIRE(strcmp((const char *)bychars, "Za\xE3\x81\x82") == 0);

  unsigned char bybytes[kBig] = {'Z', 0, 0, 0, 0, 0, 0, 0};
  REQUIRE(_mbsnbcat_s(bybytes, kBig, kMixed, 2) == 0);
  REQUIRE(strcmp((const char *)bybytes, "Za") == 0);

  // A counted append of zero with nothing to do: the reference's
  // `srclen == 0 && dst == NULL && dstsize == 0` arm, success with no
  // error, and the null/zero pair is the only one that takes it.  errno is
  // left as the previous test left it -- a face that succeeds does not
  // clear the error state, and asserting that here would pin down the one
  // behaviour the secure-crt contract does not promise.
  errno = 0;
  REQUIRE(_mbsncat_s(nullptr, 0, nullptr, 0) == 0);
  REQUIRE(errno == 0);
  errno = 0;
  REQUIRE(_mbsnbcat_s(nullptr, 0, nullptr, 0) == 0);
  REQUIRE(errno == 0);

  unsigned char buf[kBig] = "keep";
  REQUIRE(_mbsncat_s(buf, kBig, kMixed, 0) == 0);
  REQUIRE(strcmp((const char *)buf, "keep") == 0);
}

TEST_CASE("the _s copies report EILSEQ for a truncated final character")
{
  // The reference's `_ISMBBLEADPREFIX` test, widened from a two-byte dud
  // pair to every UTF-8 width (D9w): the copied region ends in a character
  // that never finishes, so the destination is left holding the prefix
  // that is one and the call reports EILSEQ.
  unsigned char buf[kBig] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  errno = 0;
  REQUIRE(_mbscpy_s(buf, kBig, kCut) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  REQUIRE(strcmp((const char *)buf, "a") == 0);

  // The same shape through the append form, with a *terminated*
  // destination -- the point of the pair is that the EILSEQ is about the
  // source's tail and not about an unterminated buffer, and only one of
  // the two faces is about each.
  memset(buf, 0, kBig);
  REQUIRE(_mbscat_s(buf, kBig, kCut) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  REQUIRE(strcmp((const char *)buf, "a") == 0);

  // Four bytes of U+1F600 that are three: the copied region is four bytes
  // long and the character in it is not one of them.  This is the case
  // the reference's `dst[-2] = '\0'` repair cannot express, because the
  // dud is four bytes wide rather than two.
  memset(buf, 'x', kBig);
  const unsigned char trunc4[] = {0xE3, 0x81, 0x82, 0xF0, 0x9F, 0};
  errno = 0;
  REQUIRE(_mbscpy_s(buf, kBig, trunc4) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  const unsigned char want_hira[] = {0xE3, 0x81, 0x82, 0};
  REQUIRE(memcmp(buf, want_hira, 4) == 0);

  // A well-formed string is not affected by any of this.
  memset(buf, 'x', kBig);
  REQUIRE(_mbscpy_s(buf, kBig, kHira2) == 0);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82\xE3\x81\x82") == 0);
}

// ---------------------------------------------------------------- set

TEST_CASE("_mbsset fills every character, per the wine anchor")
{
  // wine: _mbsset("abab", 'X') gives "XXXX".
  unsigned char dst[6] = {'a', 'b', 'a', 'b', 0, 0};
  REQUIRE(_mbsset(dst, 'X') == dst);
  REQUIRE(strcmp((const char *)dst, "XXXX") == 0);

  // A value the destination can hold character for character is written
  // as a character, not as its first byte.
  unsigned char kana[8] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x82, 0, 0};
  errno = 0;
  REQUIRE(_mbsset(kana, 0x3042) == kana);
  REQUIRE(errno == 0);
  REQUIRE(memcmp(kana, kHira2, 7) == 0);

  errno = 0;
  REQUIRE(_mbsset(nullptr, 'X') == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("a value that does not fit the character is replaced by spaces")
{
  // The reference's "invalid MBC pair" repair: it cannot write a DBCS pair
  // where a single byte sits, so it writes a space and reports EINVAL
  // rather than leaving half a pair behind.  Over UTF-8 the rule is the
  // same shape -- a character that is not the replacement's width becomes
  // as many spaces as it had bytes -- which is what keeps every
  // intermediate string decodable.
  unsigned char mixed[6] = {'a', 0xE3, 0x81, 0x82, 'b', 0};
  errno = 0;
  REQUIRE(_mbsset(mixed, 0x3042) == mixed);
  REQUIRE(errno == EINVAL);
  REQUIRE(mixed[0] == ' ');
  REQUIRE(memcmp(mixed + 1, kHira, 3) == 0);
  REQUIRE(mixed[4] == ' ');

  // ... and the result still decodes, which is the point of using spaces
  // instead of refusing.
  REQUIRE(_mbstrlen((const char *)mixed) == 3);
}

TEST_CASE("_mbsnset counts characters and _mbsnbset counts bytes")
{
  // The set family keeps the split the copy family has, which is the third
  // place in a row it could have gone the other way.
  unsigned char bychars[kBig] = "a\xE3\x81\x82z";
  errno = 0;
  REQUIRE(_mbsnset(bychars, 'X', 2) == bychars);
  REQUIRE(errno == EINVAL);
  REQUIRE(strcmp((const char *)bychars, "X   z") == 0);

  // A byte count of 4 covers "a" and the whole of U+3042, and the two uses
  // together add up to the budget exactly -- so the walk stops there and
  // the 'z' behind it is untouched.  U+3042 is blanked because the value
  // is one byte wide where the character is three, which is a different
  // reason from the budget: the budget ran out *after* it.
  unsigned char bybytes[kBig] = "a\xE3\x81\x82z";
  errno = 0;
  REQUIRE(_mbsnbset(bybytes, 'X', 4) == bybytes);
  REQUIRE(errno == EINVAL);
  REQUIRE(strcmp((const char *)bybytes, "X   z") == 0);

  // The same value against a two-byte character with a budget that ends
  // *inside* it: this is the row the reference's "pad with ' ' if no room
  // for both bytes" is about, and it stops there rather than carrying on.
  unsigned char split[kBig] = "a\xC3\xA9z";
  errno = 0;
  REQUIRE(_mbsnbset(split, 'X', 2) == split);
  REQUIRE(errno == EINVAL);
  REQUIRE(strcmp((const char *)split, "X  z") == 0);

  // On an all-ASCII string the two are the same function, which is the
  // only situation where the two faces should agree.  errno is cleared
  // first because the rows above left EINVAL in it and a successful call
  // does not clear errno on its way out.
  unsigned char plain1[kBig] = "abcd";
  unsigned char plain2[kBig] = "abcd";
  errno = 0;
  REQUIRE(_mbsnset(plain1, 'X', 2) == plain1);
  REQUIRE(_mbsnbset(plain2, 'X', 2) == plain2);
  REQUIRE(strcmp((const char *)plain1, "XXcd") == 0);
  REQUIRE(strcmp((const char *)plain2, "XXcd") == 0);
  REQUIRE(errno == 0);

  // The byte-counted face blanks the character that no longer fits rather
  // than writing one space and leaving the rest of its bytes behind, so
  // what it produces still decodes.
  unsigned char tight[kBig] = "a\xE3\x81\x82z";
  errno = 0;
  REQUIRE(_mbsnbset(tight, 'X', 3) == tight);
  REQUIRE(errno == EINVAL);
  REQUIRE(strcmp((const char *)tight, "X   z") == 0);
  REQUIRE(_mbstrlen((const char *)tight) == 5);

  errno = 0;
  REQUIRE(_mbsnset(nullptr, 'X', 0) == nullptr);
  REQUIRE(errno == 0);
  REQUIRE(_mbsnbset(nullptr, 'X', 0) == nullptr);
  REQUIRE(errno == 0);
  REQUIRE(_mbsnset(nullptr, 'X', 1) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("the set_s family has a zero-count no-op and _mbsset_s does not")
{
  // An asymmetry the reference has and the tests exist to keep: the
  // counted forms accept the null/zero/zero triple as "nothing to do",
  // the whole-string form does not, because it has no count to look at.
  REQUIRE(_mbsnset_s(nullptr, 0, 'X', 0) == 0);
  REQUIRE(_mbsnbset_s(nullptr, 0, 'X', 0) == 0);

  unsigned char buf[kBig] = "keep";
  REQUIRE(_mbsnset_s(buf, kBig, 'X', 0) == 0);
  REQUIRE(strcmp((const char *)buf, "keep") == 0);

  errno = 0;
  REQUIRE(_mbsset_s(nullptr, 0, 'X') == EINVAL);
  REQUIRE(errno == EINVAL);

  unsigned char fill[kBig] = "abcd";
  errno = 0;
  // A single-byte value that is not a lead byte is a legal mbchar, so the
  // whole-string form only refuses the null/zero pair -- unlike the
  // counted forms, which also take a zero count as "nothing to do".
  REQUIRE(_mbsset_s(fill, kBig, 'X') == 0);
  REQUIRE(strcmp((const char *)fill, "XXXX") == 0);
  REQUIRE(fill[4] == 0);

  unsigned char bychars[kBig] = "a\xE3\x81\x82z";
  errno = 0;
  REQUIRE(_mbsnset_s(bychars, kBig, 'X', 2) == EILSEQ);
  REQUIRE(errno == EILSEQ);
  REQUIRE(strcmp((const char *)bychars, "X   z") == 0);

  // A destination with no terminator inside size is EINVAL, not ERANGE:
  // the reference cannot tell an unterminated buffer from a full one.
  memset(fill, 'x', kBig);
  errno = 0;
  REQUIRE(_mbsnset_s(fill, 3, 'X', 1) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(fill[2] == 0);
}

// ------------------------------------------------------------ tokenize

TEST_CASE("_mbstok_s round-trips a string with a multi-byte token")
{
  // "a" + U+3042 + ", b": two tokens, the first of which is a three-byte
  // character.  The delimiter's first byte is blanked and the context
  // resumes one whole character later, so a token always ends on a
  // character boundary and every token decodes.
  unsigned char buf[] = "a\xE3\x81\x82, b";
  unsigned char *ctx = nullptr;
  const unsigned char *set = (const unsigned char *)",";

  unsigned char *t1 = _mbstok_s(buf, set, &ctx);
  REQUIRE(t1 == buf);
  REQUIRE(strcmp((const char *)t1, "a\xE3\x81\x82") == 0);
  REQUIRE(_mbstrlen((const char *)t1) == 2);

  unsigned char *t2 = _mbstok_s(nullptr, set, &ctx);
  REQUIRE(t2 != nullptr);
  // ',' is the only delimiter, so the space is part of the second token --
  // strtok does not skip leading delimiters that are not there, and this
  // face does not either.
  REQUIRE(strcmp((const char *)t2, " b") == 0);

  // At the end the context sits on the terminator and the enumeration is
  // over, which is the reference's `*context = ptr` before the null
  // return.  Checking that *context is "" rather than just non-null is
  // what tells the two exits apart.
  REQUIRE(_mbstok_s(nullptr, set, &ctx) == nullptr);
  REQUIRE(ctx != nullptr);
  REQUIRE(*ctx == 0);
}

TEST_CASE("_mbstok_s treats a multi-byte character as one delimiter")
{
  // U+3042 as the delimiter: the token before it and the token after it,
  // and the context points one *character* past the delimiter rather than
  // one byte, which is the whole difference between a character-aware
  // tokenizer and strtok over bytes.
  unsigned char buf[] = "a\xE3\x81\x82z";
  unsigned char *ctx = nullptr;
  const unsigned char *set = kHira;

  unsigned char *t1 = _mbstok_s(buf, set, &ctx);
  REQUIRE(t1 == buf);
  REQUIRE(strcmp((const char *)t1, "a") == 0);
  REQUIRE(ctx == buf + 4); // one past the three-byte delimiter

  unsigned char *t2 = _mbstok_s(nullptr, set, &ctx);
  REQUIRE(t2 == buf + 4);
  REQUIRE(strcmp((const char *)t2, "z") == 0);
  REQUIRE(_mbstok_s(nullptr, set, &ctx) == nullptr);
}

TEST_CASE("_mbstok_s reports every reference validation shape")
{
  unsigned char buf[] = "a,b";
  unsigned char *ctx = nullptr;
  const unsigned char *set = (const unsigned char *)",";

  // A string that is nothing but delimiters has no token at all.
  unsigned char alldelim[] = ",,";
  REQUIRE(_mbstok_s(alldelim, set, &ctx) == nullptr);
  REQUIRE(ctx == alldelim + 2);

  // A null control, a null context, and a null string with an empty
  // context are three separate rows of the reference's validation block.
  errno = 0;
  REQUIRE(_mbstok_s(buf, nullptr, &ctx) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_mbstok_s(buf, set, nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  unsigned char *empty_ctx = nullptr;
  REQUIRE(_mbstok_s(nullptr, set, &empty_ctx) == nullptr);
  REQUIRE(errno == EINVAL);

  // An empty charset means no delimiters, so the whole string is one
  // token and the next call is the end.
  unsigned char whole[] = "a\xE3\x81\x82,b";
  unsigned char *wctx = nullptr;
  unsigned char *w = _mbstok_s(whole, (const unsigned char *)"", &wctx);
  REQUIRE(w == whole);
  REQUIRE(strcmp((const char *)w, "a\xE3\x81\x82,b") == 0);
  REQUIRE(_mbstok_s(nullptr, (const unsigned char *)"", &wctx) == nullptr);
}

TEST_CASE("_mbstok keeps its own static cursor")
{
  // The reference's secure form threads the caller's context through and
  // never looks at the non-secure form's static, so the two enumerations
  // are independent -- and the non-secure cursor has to be drained before
  // the test leaves, or the next test to call _mbstok inherits it.  The
  // token after the delimiter is " b", space and all: the space is not in
  // the charset, so it is the first character of the second token.
  unsigned char buf[] = "a\xE3\x81\x82, b";
  unsigned char *first = _mbstok(buf, (const unsigned char *)",");
  REQUIRE(first == buf);
  REQUIRE(strcmp((const char *)first, "a\xE3\x81\x82") == 0);
  unsigned char *second = _mbstok(nullptr, (const unsigned char *)",");
  REQUIRE(second != nullptr);
  REQUIRE(strcmp((const char *)second, " b") == 0);
  REQUIRE(_mbstok(nullptr, (const unsigned char *)",") == nullptr);

  // The secure form's cursor is the caller's, so it starts at the
  // beginning of its own string regardless of where _mbstok left off.
  unsigned char other[] = "1,2";
  unsigned char *ctx = nullptr;
  unsigned char *s = _mbstok_s(other, (const unsigned char *)",", &ctx);
  REQUIRE(s == other);
  REQUIRE(strcmp((const char *)s, "1") == 0);
}

// --------------------------------------------------------- locale twins

TEST_CASE("every writing face ignores a non-null locale")
{
  const _locale_t anywhere = (_locale_t)0x1234;

  unsigned char buf[kBig];
  memset(buf, 0, kBig);
  REQUIRE(_mbsncpy_l(buf, kHira2x, 2, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82\xE3\x81\x82") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsnbcpy_l(buf, kHira2x, 2, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsncat_l(buf, kHira2x, 1, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsnbcat_l(buf, kHira2x, 1, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbscpy_s_l(buf, kBig, kHira, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsncpy_s_l(buf, kBig, kHira2x, 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsnbcpy_s_l(buf, kBig, kHira2x, 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsncat_s_l(buf, kBig, kHira2x, 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "\xE3\x81\x82") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbsnbcat_s_l(buf, kBig, kHira2x, 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "") == 0);

  memset(buf, 0, kBig);
  REQUIRE(_mbscpy_s_l(buf, kBig, nullptr, anywhere) == EINVAL);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsnset_l(buf, 'X', 1, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsnbset_l(buf, 'X', 1, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsset_l(buf, 'X', anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsset_s_l(buf, kBig, 'X', anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsnset_s_l(buf, kBig, 'X', 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsnbset_s_l(buf, kBig, 'X', 1, anywhere) == 0);
  REQUIRE(strcmp((const char *)buf, "X") == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbslwr_l(buf, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "a") == 0);
  REQUIRE(_mbslwr_s_l(buf, kBig, anywhere) == 0);

  memset(buf, 'a', kBig);
  buf[1] = 0;
  REQUIRE(_mbsupr_l(buf, anywhere) == buf);
  REQUIRE(strcmp((const char *)buf, "A") == 0);
  REQUIRE(_mbsupr_s_l(buf, kBig, anywhere) == 0);

  unsigned char rev[] = {'a', 0xE3, 0x81, 0x82, 'z', 0};
  REQUIRE(_mbsrev_l(rev, anywhere) == rev);
  const unsigned char want_rev[] = {'z', 0xE3, 0x81, 0x82, 'a', 0};
  REQUIRE(memcmp(rev, want_rev, 6) == 0);

  unsigned char tok[] = "a\xE3\x81\x82,b";
  unsigned char *ctx = nullptr;
  REQUIRE(_mbstok_s_l(tok, (const unsigned char *)",", &ctx, anywhere) == tok);
  REQUIRE(strcmp((const char *)tok, "a\xE3\x81\x82") == 0);
  // The 'b' behind the delimiter is a second token, so the enumeration is
  // not over yet; the null comes on the call after that.
  unsigned char *rest =
      _mbstok_s_l(nullptr, (const unsigned char *)",", &ctx, anywhere);
  REQUIRE(rest != nullptr);
  REQUIRE(strcmp((const char *)rest, "b") == 0);
  REQUIRE(_mbstok_s_l(nullptr, (const unsigned char *)",", &ctx, anywhere) ==
          nullptr);

  unsigned char stat[] = "a\xE3\x81\x82,b";
  REQUIRE(_mbstok_l(stat, (const unsigned char *)",", anywhere) == stat);
  REQUIRE(strcmp((const char *)stat, "a\xE3\x81\x82") == 0);
  REQUIRE(_mbstok_l(nullptr, (const unsigned char *)",", anywhere) != nullptr);
  REQUIRE(_mbstok_l(nullptr, (const unsigned char *)",", anywhere) == nullptr);
}
