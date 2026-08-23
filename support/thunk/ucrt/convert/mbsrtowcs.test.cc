#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 mbsrtowcs")
{
  const char in[] = "你a😀";
  wchar_t dst[16];

  for (auto &w : dst)
    w = wchar_t(0xcccc);

  SECTION("full conversion writes the terminator")
  {
    const char *p = in;
    size_t r = mbsrtowcs(dst, &p, 15, nullptr);
    REQUIRE(r == 4);
    REQUIRE(dst[0] == L'你');
    REQUIRE(dst[1] == L'a');
    REQUIRE(dst[2] == 0xd83d); // surrogate halves of 😀
    REQUIRE(dst[3] == 0xde00);
    REQUIRE(dst[4] == L'\0');
    REQUIRE(p == nullptr);
  }

  SECTION("too small stops before the pair, partial count")
  {
    const char *p = in;
    size_t r = mbsrtowcs(dst, &p, 3, nullptr); // 你 + a fit, the pair does not
    REQUIRE(r == 2);
    REQUIRE(p == in + 4);
    REQUIRE(dst[2] == wchar_t(0xcccc)); // nothing written for the pair
  }

  SECTION("exact fit leaves the NUL unconsumed")
  {
    const char fit[] = "你a";
    const char *p = fit;
    size_t r = mbsrtowcs(dst, &p, 2, nullptr);
    REQUIRE(r == 2);
    REQUIRE(p == fit + 4);
    REQUIRE(*p == '\0');
    REQUIRE(dst[2] == wchar_t(0xcccc)); // no room for the terminator
  }

  SECTION("invalid input reports EILSEQ")
  {
    const char bad[] = "\xE0\x80\x80"; // overlong encoding, no character form
    const char *p = bad;
    errno = 0;
    size_t r = mbsrtowcs(dst, &p, 15, nullptr);
    REQUIRE(r == size_t(-1));
    REQUIRE(errno == EILSEQ);
    REQUIRE(p == bad);
  }
}
