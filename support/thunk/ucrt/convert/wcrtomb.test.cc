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
