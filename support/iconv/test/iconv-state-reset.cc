#include <catch_amalgamated.hpp>

#include <iconv.h>

using namespace std;

TEST_CASE("iconv state reset")
{
  iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  const char *input = "AB";
  size_t inbytesleft = 2;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  REQUIRE(result != (size_t)-1);

  // Reset conversion state by passing NULL for inbuf
  result = iconv(cd, NULL, NULL, &outptr, &outbytesleft);
  iconv_close(cd);

  REQUIRE(result == 0);
}
