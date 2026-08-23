#include <catch_amalgamated.hpp>

#include <ctype.h>
#include <locale.h>
#include <wchar.h>

#undef _tolower
#undef _toupper
extern "C" int __cdecl _tolower_l(int c, _locale_t locale);
extern "C" int __cdecl _toupper_l(int c, _locale_t locale);

TEST_CASE("tolower toupper")
{
  // checked folds (wine anchors)
  REQUIRE(tolower('A') == 'a');
  REQUIRE(tolower('a') == 'a');
  REQUIRE(tolower('1') == '1');
  REQUIRE(tolower(0xE9) == 0xE9); // >= 0x80 untouched
  REQUIRE(tolower(-1) == -1);     // EOF passthrough
  REQUIRE(tolower(0x4F60) == 0x4F60);

  REQUIRE(toupper('a') == 'A');
  REQUIRE(toupper('A') == 'A');
  REQUIRE(toupper('1') == '1');
  REQUIRE(toupper(0xC9) == 0xC9);
  REQUIRE(toupper(-1) == -1);

  // full ASCII sweep
  for (int c = 0; c < 256; c++) {
    int expected = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    REQUIRE(tolower(c) == expected);
    expected = (c >= 'a' && c <= 'z') ? c - 32 : c;
    REQUIRE(toupper(c) == expected);
  }
}

TEST_CASE("_tolower _toupper raw folds")
{
  // wine anchors: raw arithmetic, no validation, no truncation
  REQUIRE(_tolower('A') == 97);
  REQUIRE(_tolower('a') == 129);
  REQUIRE(_tolower('1') == 81);
  REQUIRE(_tolower(0xE9) == 265);
  REQUIRE(_tolower(-1) == 31);

  REQUIRE(_toupper('a') == 65);
  REQUIRE(_toupper('A') == 33);
  REQUIRE(_toupper('1') == 17);
  REQUIRE(_toupper(0xC9) == 169);
  REQUIRE(_toupper(-1) == -33);

  // _l variants: locale ignored, same raw semantics
  REQUIRE(_tolower_l('A', nullptr) == 97);
  REQUIRE(_tolower_l('a', nullptr) == 129);
  REQUIRE(_toupper_l('a', nullptr) == 65);
  REQUIRE(_toupper_l('A', nullptr) == 33);
}

TEST_CASE("fold pollution adversarial")
{
  // Under a CP1252-capable native locale the native tolower folds
  // 0xC9 (E-acute) down to 0xE9; ours stays byte-transparent
  _wsetlocale(LC_CTYPE, L"french");
  REQUIRE(tolower(0xC9) == 0xC9);
  REQUIRE(toupper(0xE9) == 0xE9);
  REQUIRE(tolower('A') == 'a');
  _wsetlocale(LC_CTYPE, L"C");
}
