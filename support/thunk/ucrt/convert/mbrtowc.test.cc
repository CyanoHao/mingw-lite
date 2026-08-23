#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

TEST_CASE("UTF-8 mbrtowc")
{
  wchar_t w = 1;

  SECTION("non-BMP degrades to the replacement character")
  {
    errno = 0;
    REQUIRE(mbrtowc(&w, "😀", 4, nullptr) == 4);
    REQUIRE(w == L'�'); // a surrogate pair does not fit a single wchar
    REQUIRE(errno == 0); // the degradation is not an error
  }

  SECTION("multi-byte BMP")
  {
    REQUIRE(mbrtowc(&w, "你", 3, nullptr) == 3);
    REQUIRE(w == L'你');
  }

  SECTION("invalid input reports EILSEQ")
  {
    errno = 0;
    REQUIRE(mbrtowc(&w, "\xE0\x80\x80", 3, nullptr) ==
            size_t(-1)); // overlong encoding, no character form
    REQUIRE(errno == EILSEQ);
  }

  SECTION("null source probes the state")
  {
    REQUIRE(mbrtowc(&w, nullptr, 0, nullptr) == 0);
  }
}
