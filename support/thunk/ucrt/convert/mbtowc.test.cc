#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 mbtowc")
{
  wchar_t w = 1;
  REQUIRE(mbtowc(&w, "😀", 4) == 4); // full byte count
  REQUIRE(w == L'�');                // single-wchar degradation
  REQUIRE(mbtowc(nullptr, nullptr, 0) == 0);
}
