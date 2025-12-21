#include <catch_amalgamated.hpp>

#include <windows.h>

TEST_CASE("GetOEMCP")
{
  REQUIRE(GetOEMCP() == CP_UTF8);
}
