#include <catch_amalgamated.hpp>

#include <errno.h>
#include <uchar.h>
#include <wchar.h>

// Overlay-bound C11 char16/char32 conversions.  wine's ucrtbase
// exports all four as unimplemented stubs (M7 finding), so these
// assert the C11/musl state machines directly.
extern "C" size_t __cdecl mbrtoc16(char16_t *pc16, const char *s, size_t n,
                                   mbstate_t *ps);
extern "C" size_t __cdecl c16rtomb(char *s, char16_t c16, mbstate_t *ps);
extern "C" size_t __cdecl mbrtoc32(char32_t *pc32, const char *s, size_t n,
                                   mbstate_t *ps);
extern "C" size_t __cdecl c32rtomb(char *s, char32_t c32, mbstate_t *ps);

TEST_CASE("mbrtoc16")
{
  char16_t c16;
  mbstate_t st;

  memset(&st, 0, sizeof st);
  REQUIRE(mbrtoc16(&c16, "A", 1, &st) == 1);
  REQUIRE(c16 == u'A');

  // non-BMP splits: first call consumes 4 bytes, yields the high half
  memset(&st, 0, sizeof st);
  REQUIRE(mbrtoc16(&c16, "\xf0\x9f\x98\x80", 4, &st) == 4);
  REQUIRE(c16 == 0xd83d);
  // pending half returned as (size_t)-3, no input consumed
  REQUIRE(mbrtoc16(&c16, "", 0, &st) == (size_t)-3);
  REQUIRE(c16 == 0xde00);

  // null ps: internal state, single clean call
  REQUIRE(mbrtoc16(&c16, "B", 1, nullptr) == 1);
  REQUIRE(c16 == u'B');
}

TEST_CASE("c16rtomb")
{
  char b[8];
  mbstate_t st;

  // surrogate pair: 0 while pending, then the 4-byte UTF-8 encoding
  memset(&st, 0, sizeof st);
  REQUIRE(c16rtomb(b, 0xd83d, &st) == 0);
  REQUIRE(c16rtomb(b, 0xde00, &st) == 4);
  REQUIRE((unsigned char)b[0] == 0xf0);
  REQUIRE((unsigned char)b[1] == 0x9f);
  REQUIRE((unsigned char)b[2] == 0x98);
  REQUIRE((unsigned char)b[3] == 0x80);

  // lone low surrogate with a clean state: EILSEQ
  memset(&st, 0, sizeof st);
  errno = 0;
  REQUIRE(c16rtomb(b, 0xdc00, &st) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // high surrogate followed by a non-low unit: EILSEQ
  memset(&st, 0, sizeof st);
  REQUIRE(c16rtomb(b, 0xd83d, &st) == 0);
  errno = 0;
  REQUIRE(c16rtomb(b, u'x', &st) == (size_t)-1);
  REQUIRE(errno == EILSEQ);
}

TEST_CASE("mbrtoc32")
{
  char32_t c32;
  mbstate_t st;

  memset(&st, 0, sizeof st);
  REQUIRE(mbrtoc32(&c32, "\xe4\xbd\xa0", 3, &st) == 3);
  REQUIRE(c32 == 0x4F60);

  // surrogate-encoded (CESU-8) input is invalid UTF-8: EILSEQ
  memset(&st, 0, sizeof st);
  errno = 0;
  REQUIRE(mbrtoc32(&c32, "\xed\xa0\x80", 3, &st) == (size_t)-1);
  REQUIRE(errno == EILSEQ);

  // truncated sequence: INCOMPLETE (-2)
  memset(&st, 0, sizeof st);
  REQUIRE(mbrtoc32(&c32, "\xe4\xbd", 2, &st) == (size_t)-2);
}

TEST_CASE("c32rtomb")
{
  char b[8];
  mbstate_t st;

  memset(&st, 0, sizeof st);
  REQUIRE(c32rtomb(b, 0x41, &st) == 1);
  REQUIRE(b[0] == 'A');

  memset(&st, 0, sizeof st);
  REQUIRE(c32rtomb(b, 0x1F600, &st) == 4);
  REQUIRE((unsigned char)b[0] == 0xf0);
  REQUIRE((unsigned char)b[3] == 0x80);

  // null s with an initial state: 1 (C11 shape, musl parity)
  memset(&st, 0, sizeof st);
  REQUIRE(c32rtomb(nullptr, 0x41, &st) == 1);
}
