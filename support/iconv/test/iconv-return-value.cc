#include <catch_amalgamated.hpp>

#include <iconv.h>

using namespace std;

TEST_CASE("iconv return value - all reversible")
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
  iconv_close(cd);

  REQUIRE(result != (size_t)-1);
  REQUIRE(result == 0);
  REQUIRE(inbytesleft == 0);
}

TEST_CASE("iconv return value - unrepresentable characters")
{
  // Convert from ISO-8859-15 (contains Euro sign € at 0xA4)
  // to ISO-8859-1 (Euro sign not available)
  //
  // Without //TRANSLIT or //IGNORE, should fail with EILSEQ
  iconv_t cd = iconv_open("ISO-8859-1", "ISO-8859-15");
  REQUIRE(cd != (iconv_t)-1);

  // Euro sign in ISO-8859-15 is 0xA4
  const char input[] = "\xA4";  // € in ISO-8859-15
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should fail with EILSEQ for unrepresentable character
  REQUIRE(result == (size_t)-1);
  REQUIRE(errno == EILSEQ);
}
