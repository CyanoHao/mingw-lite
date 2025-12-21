#include <catch_amalgamated.hpp>

#include <windows.h>

TEST_CASE("GetACP")
{
  REQUIRE(GetACP() == CP_UTF8);
}
