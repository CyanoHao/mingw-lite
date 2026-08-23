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

TEST_CASE("mblen")
{
  // mblen == mbtowc(nullptr, s, n): shared UTF-8 engine and state
  REQUIRE(mblen("a", 1) == 1);                     // ASCII
  REQUIRE(mblen("\xe4\xbd\xa0", 3) == 3);          // 你, full length
  REQUIRE(mblen("\xe4\xbd\xa0", 2) == -1);         // truncated sequence
  REQUIRE(mblen("", 1) == 0);                      // empty string
  REQUIRE(mblen(nullptr, 0) == 0);                 // state probe
  REQUIRE(mblen("\xf0\x9f\x98\x80", 4) == 4);      // 4-byte emoji
  REQUIRE(mblen("\xed\xa0\x80", 3) == -1);         // surrogate half
}
