#include <catch_amalgamated.hpp>

#include <stddef.h>
#include <string.h>

// The two pseudo-tables.  Spelled out here rather than through
// <mbctype.h>, which #defines `_mbcasemap` to `__p__mbcasemap()` and so
// would never let the data export be named -- and the bit values are
// spelled as numbers for the same reason the M13a tests spell them: the
// header's names are not guaranteed to be reachable.
extern "C"
{
  unsigned char *__cdecl __p__mbctype(void);
  unsigned char *__cdecl __p__mbcasemap(void);
  // The `extern` is load-bearing: without it this line *defines* a
  // zero-filled array in the test's own object, which then wins over the
  // overlay's export at link time and every assertion below quietly reads
  // a table of zeros instead of the one under test.
  extern unsigned char _mbcasemap[256];
}

namespace
{
  constexpr unsigned char kSBUp = 0x10;
  constexpr unsigned char kSBLow = 0x20;
  constexpr unsigned char kM1 = 0x04; // a UTF-8 lead byte
  constexpr unsigned char kM2 = 0x08; // a UTF-8 trail byte
} // namespace

TEST_CASE("__p__mbctype hands out the character-type bit table")
{
  const unsigned char *t = __p__mbctype();
  REQUIRE(t != nullptr);
  // The same table every call: it is a static, not a fresh copy.
  REQUIRE(__p__mbctype() == t);

  // Index 0 is the reference's EOF slot and is empty, like every byte
  // that carries no information here.
  REQUIRE(t[0] == 0);
  REQUIRE(t['0'] == 0);
  REQUIRE(t[' '] == 0);

  // ASCII letters get the upper/lower bits the reference's setSBUpLow
  // gives them -- the bits its own macros read.
  REQUIRE(t['A'] == kSBUp);
  REQUIRE(t['Z'] == kSBUp);
  REQUIRE(t['a'] == kSBLow);
  REQUIRE(t['z'] == kSBLow);

  // The UTF-8 byte roles: 0x80..0xBF can only be a continuation byte,
  // 0xC2..0xF4 can only be a lead.  The two bytes between them (C0, C1)
  // begin an overlong form and the range above F4 begins nothing, so
  // they carry no bit and a reader of the table calls them illegal.
  REQUIRE(t[0x80] == kM2);
  REQUIRE(t[0xBF] == kM2);
  REQUIRE(t[0xC0] == 0);
  REQUIRE(t[0xC1] == 0);
  REQUIRE(t[0xC2] == kM1);
  REQUIRE(t[0xF4] == kM1);
  REQUIRE(t[0xF5] == 0);
  REQUIRE(t[0xFF] == 0);
}

TEST_CASE("the character-type table is the expected byte for every index")
{
  // A sweep rather than samples, because the table is a literal and the
  // one thing that can be wrong with a literal is a value in the middle
  // of a run.  The expectation is computed from the rule, so the table
  // and the rule have to agree everywhere.
  const unsigned char *t = __p__mbctype();
  REQUIRE(t != nullptr);

  for (unsigned int b = 0; b < 256; ++b) {
    INFO("byte 0x" << std::hex << b);
    unsigned char want = 0;
    if (b >= 0x41 && b <= 0x5A)
      want |= kSBUp;
    if (b >= 0x61 && b <= 0x7A)
      want |= kSBLow;
    if (b >= 0x80 && b <= 0xBF)
      want |= kM2;
    if (b >= 0xC2 && b <= 0xF4)
      want |= kM1;
    REQUIRE(t[b] == want);
  }

  // The 257th entry exists because the table's documented extent is 257;
  // nothing indexes it for a byte value.
  REQUIRE(t[256] == 0);
}

TEST_CASE("__p__mbcasemap and _mbcasemap are the same bytes")
{
  // Two names for one table: the data export is what a caller that
  // declares `extern unsigned char _mbcasemap[]` reads, and the pointer
  // is what the header's macro hands out.  If they were two tables, a
  // program mixing the two spellings would see two different answers.
  REQUIRE(__p__mbcasemap() == _mbcasemap);
  REQUIRE(__p__mbcasemap() == __p__mbcasemap());
}

TEST_CASE("the case table maps the letters and nothing else")
{
  const unsigned char *m = __p__mbcasemap();
  REQUIRE(m != nullptr);

  // The reference's _MBCASEMAP_DEFAULT is a case *swap*: both
  // directions are in the one table, which is why a single object
  // serves _mbbtolower and _mbbtoupper alike.
  REQUIRE(m['A'] == 'a');
  REQUIRE(m['Z'] == 'z');
  REQUIRE(m['a'] == 'A');
  REQUIRE(m['z'] == 'Z');

  // Everything else is the reference's zero -- "no mapping is recorded
  // here" -- and not an identity.  The table is only read for a letter.
  REQUIRE(m['0'] == 0);
  REQUIRE(m['@'] == 0);
  REQUIRE(m[0x00] == 0);
  REQUIRE(m[0x80] == 0);
  REQUIRE(m[0xFF] == 0);
}

TEST_CASE("the case table is the expected byte for every index")
{
  const unsigned char *m = __p__mbcasemap();
  REQUIRE(m != nullptr);

  for (unsigned int b = 0; b < 256; ++b) {
    INFO("byte 0x" << std::hex << b);
    unsigned char want = 0;
    if (b >= 'A' && b <= 'Z')
      want = (unsigned char)(b + 0x20);
    else if (b >= 'a' && b <= 'z')
      want = (unsigned char)(b - 0x20);
    REQUIRE(m[b] == want);
  }
}
