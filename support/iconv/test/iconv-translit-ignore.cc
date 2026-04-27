#include <catch_amalgamated.hpp>

#include <iconv.h>

using namespace std;

TEST_CASE("iconv //TRANSLIT - unrepresentable character replacement")
{
  // //TRANSLIT should replace unconvertible characters with '?'
  // (Win32 API compatible behavior)
  iconv_t cd = iconv_open("ISO-8859-1//TRANSLIT", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  // Euro sign (€) cannot be represented in ISO-8859-1
  const char input[] = "\xe2\x82\xac";  // € in UTF-8
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should succeed with 1 non-reversible conversion
  REQUIRE(result != (size_t)-1);
  REQUIRE(result == 1);
  REQUIRE(inbytesleft == 0);

  // Output should be '?'
  size_t output_len = outptr - output;
  REQUIRE(output_len == 1);
  REQUIRE(output[0] == '?');
}

TEST_CASE("iconv //IGNORE - skip unconvertible characters")
{
  // //IGNORE should skip unconvertible characters
  iconv_t cd = iconv_open("ISO-8859-1//IGNORE", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  // Euro sign (€) cannot be represented in ISO-8859-1
  const char input[] = "A\xe2\x82\xac" "B";  // A€B in UTF-8
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should succeed with 1 non-reversible conversion (the skipped €)
  REQUIRE(result != (size_t)-1);
  REQUIRE(result == 1);
  REQUIRE(inbytesleft == 0);

  // Output should be "AB" (€ skipped)
  size_t output_len = outptr - output;
  REQUIRE(output_len == 2);
  REQUIRE(output[0] == 'A');
  REQUIRE(output[1] == 'B');
}

TEST_CASE("iconv strict mode - EILSEQ for unconvertible characters")
{
  // Without //TRANSLIT or //IGNORE, should fail with EILSEQ
  iconv_t cd = iconv_open("ISO-8859-1", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  // Euro sign (€) cannot be represented in ISO-8859-1
  const char input[] = "\xe2\x82\xac";  // € in UTF-8
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should fail with EILSEQ
  REQUIRE(result == (size_t)-1);
  REQUIRE(errno == EILSEQ);
}

TEST_CASE("iconv //TRANSLIT - ASCII target with UTF-8 input")
{
  // Test conversion to ASCII with //TRANSLIT
  iconv_t cd = iconv_open("ASCII//TRANSLIT", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  // "Hello 世界" - world cannot be represented in ASCII
  const char input[] = "Hello \xe4\xb8\x96\xe7\x95\x8c";
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should succeed with non-reversible conversions
  REQUIRE(result != (size_t)-1);
  REQUIRE(result > 0);  // At least 2 characters (世 and 界)
  REQUIRE(inbytesleft == 0);

  // Output should be "Hello ??" (each CJK char replaced with '?')
  size_t output_len = outptr - output;
  REQUIRE(output_len == 8);  // "Hello" (5) + " ??" (2) + null (1)
  REQUIRE(string(output, output_len) == "Hello ??");
}

TEST_CASE("iconv //IGNORE - CJK characters skipped")
{
  // Test conversion to ASCII with //IGNORE
  iconv_t cd = iconv_open("ASCII//IGNORE", "UTF-8");
  REQUIRE(cd != (iconv_t)-1);

  // "Hello 世界" - world cannot be represented in ASCII
  const char input[] = "Hello \xe4\xb8\x96\xe7\x95\x8c";
  size_t inbytesleft = sizeof(input) - 1;
  char output[32];
  char *outptr = output;
  size_t outbytesleft = sizeof(output);
  const char *inptr = input;

  size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
  iconv_close(cd);

  // Should succeed with non-reversible conversions
  REQUIRE(result != (size_t)-1);
  REQUIRE(result == 2);  // 2 characters skipped
  REQUIRE(inbytesleft == 0);

  // Output should be "Hello " (CJK chars skipped)
  size_t output_len = outptr - output;
  REQUIRE(output_len == 6);  // "Hello " (6 bytes)
  REQUIRE(string(output, output_len) == "Hello ");
}
