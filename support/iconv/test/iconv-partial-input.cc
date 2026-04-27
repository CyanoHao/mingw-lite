#include <catch_amalgamated.hpp>

#include <iconv.h>
#include <string_view>

using namespace std;

TEST_CASE("iconv partial input maintains state")
{
  iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  char output[32];
  size_t total_out = 0;
  char *outptr = output;
  size_t outleft = sizeof(output);

  // First chunk: "Hel"
  const char *input1 = "Hel";
  size_t inleft1 = 3;
  const char *inptr1 = input1;
  size_t result1 = iconv(cd, &inptr1, &inleft1, &outptr, &outleft);
  REQUIRE(result1 != (size_t)-1);
  REQUIRE(inleft1 == 0);
  total_out = sizeof(output) - outleft;

  // Second chunk: "lo"
  const char *input2 = "lo";
  size_t inleft2 = 2;
  const char *inptr2 = input2;
  size_t result2 = iconv(cd, &inptr2, &inleft2, &outptr, &outleft);
  REQUIRE(result2 != (size_t)-1);
  REQUIRE(inleft2 == 0);
  total_out = sizeof(output) - outleft;

  iconv_close(cd);

  // Verify the combined output is correct
  REQUIRE(total_out == 10);
  char expected[] = {
      0x48, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00};
  REQUIRE(string_view(expected, sizeof(expected)) ==
          string_view(output, total_out));
}
