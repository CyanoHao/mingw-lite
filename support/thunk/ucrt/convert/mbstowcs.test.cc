#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 mbstowcs")
{
  wchar_t dst[16];

  SECTION("too-small destination returns the partial count")
  {
    for (auto &w : dst)
      w = wchar_t(0xcccc);
    size_t r = mbstowcs(dst, "你a😀", 3);
    REQUIRE(r == 2); // UCRT: partial count, not (size_t)-1
  }

  SECTION("counting")
  {
    REQUIRE(mbstowcs(nullptr, "你a😀", 0) == 4);
  }
}
