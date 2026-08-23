#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 wcstombs")
{
  const wchar_t in[] = L"你a😀";
  char dst[32];

  SECTION("full conversion writes the terminator")
  {
    memset(dst, 0xcc, sizeof dst);
    size_t r = wcstombs(dst, in, 31);
    REQUIRE(r == 8); // 3 + 1 + 4 bytes
    REQUIRE((unsigned char)dst[4] == 0xf0);
    REQUIRE((unsigned char)dst[5] == 0x9f);
    REQUIRE((unsigned char)dst[6] == 0x98);
    REQUIRE((unsigned char)dst[7] == 0x80);
    REQUIRE(dst[8] == '\0');
  }

  SECTION("too small returns the partial count")
  {
    memset(dst, 0xcc, sizeof dst);
    size_t r = wcstombs(dst, in, 5); // 你 + a fit, the pair does not
    REQUIRE(r == 4);
  }

  SECTION("lone high surrogate is EILSEQ")
  {
    const wchar_t bad[] = {0xd83d, 0}; // no character form
    char out[8];
    errno = 0;
    REQUIRE(wcstombs(out, bad, 7) == size_t(-1));
    REQUIRE(errno == EILSEQ);
  }

  SECTION("lone low surrogate is EILSEQ")
  {
    const wchar_t bad[] = {0xde00, 0}; // no character form
    char out[8];
    errno = 0;
    REQUIRE(wcstombs(out, bad, 7) == size_t(-1));
    REQUIRE(errno == EILSEQ);
  }

  SECTION("counting")
  {
    REQUIRE(wcstombs(nullptr, in, 0) == 8);
  }
}
