#include <catch_amalgamated.hpp>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <string>

// Stream-channel bridge tests (musl layer, headless): real native FILE*
// streams from tmpfile() drive both directions.  The IAT-level shells
// arrive with M2.
#include <thunk/u8crt/musl.h>

namespace
{
  namespace musl = mingw_thunk::musl;

  int write_native(FILE *fp, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    int r = musl::vfprintf_to_native(fp, fmt, ap);
    va_end(ap);
    return r;
  }

  int scan_native(FILE *fp, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    int r = musl::vfscanf_from_native(fp, fmt, ap);
    va_end(ap);
    return r;
  }

  std::string slurp(FILE *fp)
  {
    std::string out;
    rewind(fp);
    int c;
    while ((c = fgetc(fp)) != EOF)
      out.push_back((char)c);
    return out;
  }
} // namespace

TEST_CASE("native bridge write side")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);

  REQUIRE(write_native(fp, "%s=%d", "k", 9) == 3);

  // interleaving with direct native writes keeps a single ordered stream
  fputc('!', fp);
  write_native(fp, "-%ls", L"你好");
  fputs("end", fp);

  fflush(fp);
  REQUIRE(slurp(fp) == "k=9!-\xe4\xbd\xa0\xe5\xa5\xbd"
                       "end");
  fclose(fp);
}

TEST_CASE("native bridge read side")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);
  fputs("10 hello 2.5 X", fp);
  rewind(fp);

  int a;
  char s[16];
  double d;
  REQUIRE(scan_native(fp, "%d %s %lf", &a, s, &d) == 3);
  REQUIRE(a == 10);
  REQUIRE(strcmp(s, "hello") == 0);
  REQUIRE(d == 2.5);

  // teardown invariant: the native stream continues at the exact byte
  // the scanf logical position stopped at
  REQUIRE(fgetc(fp) == ' ');
  REQUIRE(fgetc(fp) == 'X');
  REQUIRE(fgetc(fp) == EOF);
  fclose(fp);
}

TEST_CASE("native bridge read lookahead pushback")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);
  fputs("abc,rest", fp);
  rewind(fp);

  // a scanset consumes the delimiter via shgetc then retreats: the
  // single lookahead byte must come back through the native FILE
  char s[8];
  REQUIRE(scan_native(fp, "%[^,]", s) == 1);
  REQUIRE(strcmp(s, "abc") == 0);
  REQUIRE(fgetc(fp) == ',');
  REQUIRE(fgetc(fp) == 'r');
  REQUIRE(fgetc(fp) == 'e');
  fclose(fp);
}

TEST_CASE("native bridge read respects seek position and EOF")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);
  fputs("12345", fp);
  REQUIRE(fseek(fp, 2, SEEK_SET) == 0);

  int a;
  REQUIRE(scan_native(fp, "%d", &a) == 1);
  REQUIRE(a == 345);

  // exhausted native stream scans as EOF
  REQUIRE(scan_native(fp, "%d", &a) == -1);
  fclose(fp);
}

TEST_CASE("native bridge write-then-read roundtrip")
{
  FILE *fp = tmpfile();
  REQUIRE(fp);

  write_native(fp, "%d %s %ls", 7, "word", L"你好");
  rewind(fp);

  int a;
  char s[16];
  REQUIRE(scan_native(fp, "%d %s", &a, s) == 2);
  REQUIRE(a == 7);
  REQUIRE(strcmp(s, "word") == 0);

  // the UTF-8 bytes rendered by %ls come back byte-exact
  char utf8[16];
  REQUIRE(scan_native(fp, "%s", utf8) == 1);
  REQUIRE(strcmp(utf8, "\xe4\xbd\xa0\xe5\xa5\xbd") == 0);
  fclose(fp);
}
