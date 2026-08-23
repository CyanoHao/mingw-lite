#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 wcrtomb")
{
  char buf[8];

  SECTION("BMP")
  {
    REQUIRE(wcrtomb(buf, L'你', nullptr) == 3);
    REQUIRE((unsigned char)buf[0] == 0xe4);
  }

  SECTION("null character")
  {
    REQUIRE(wcrtomb(buf, L'\0', nullptr) == 1);
    REQUIRE(buf[0] == '\0');
  }

  SECTION("lone surrogate is EILSEQ")
  {
    errno = 0;
    REQUIRE(wcrtomb(buf, 0xd83d, nullptr) ==
            size_t(-1)); // lone high surrogate, no character form
    REQUIRE(errno == EILSEQ);
  }
}

TEST_CASE("wctomb")
{
  // wctomb == wcrtomb with a private static state, folded to int
  char buf[8];

  REQUIRE(wctomb(buf, 'a') == 1);
  REQUIRE(buf[0] == 'a');

  REQUIRE(wctomb(buf, 0x4f60) == 3); // 你, exact bytes
  REQUIRE((unsigned char)buf[0] == 0xe4);
  REQUIRE((unsigned char)buf[1] == 0xbd);
  REQUIRE((unsigned char)buf[2] == 0xa0);

  // the 4-byte path is structurally unreachable through wctomb: the
  // parameter is a 16-bit wchar_t, so no code point above U+FFFF can
  // enter (supplementary planes belong to the wcrtomb/c32rtomb
  // thunks).  U+FFFF itself encodes fine.
  REQUIRE(wctomb(buf, 0xffff) == 3);
  REQUIRE((unsigned char)buf[0] == 0xef);

  errno = 0;
  REQUIRE(wctomb(buf, 0xd83d) == -1); // lone surrogate (M0.1 parity)
  REQUIRE(errno == EILSEQ);

  REQUIRE(wctomb(buf, L'\0') == 1);
  REQUIRE(buf[0] == '\0');
}
