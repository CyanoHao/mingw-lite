#include <catch_amalgamated.hpp>

#include <ctype.h>
#include <locale.h>
#include <wchar.h>

// ctype.h function-object macros would shadow the thunk calls
#undef _tolower
#undef _toupper
#undef __iscsym
#undef __iscsymf
#undef _isalnum_l
#undef _isalpha_l
#undef _isblank_l
#undef _iscntrl_l
#undef _isdigit_l
#undef _isgraph_l
#undef _islower_l
#undef _isprint_l
#undef _ispunct_l
#undef _isspace_l
#undef _isupper_l
#undef _isxdigit_l

extern "C" int __cdecl _isctype(int c, int mask);
extern "C" int __cdecl _isctype_l(int c, int mask, _locale_t locale);
extern "C" int __cdecl _isalpha_l(int c, _locale_t locale);
extern "C" int __cdecl _isblank_l(int c, _locale_t locale);
extern "C" int __cdecl _isxdigit_l(int c, _locale_t locale);

TEST_CASE("ctype narrow table oracle")
{
  // Full 0..255 sweep against an independently computed ASCII oracle:
  // >= 0x80 must never classify (UTF-8 bytes are not characters)
  for (int c = 0; c < 256; c++) {
    bool upper = c >= 'A' && c <= 'Z';
    bool lower = c >= 'a' && c <= 'z';
    bool digit = c >= '0' && c <= '9';
    bool space = c == ' ' || (c >= '\t' && c <= '\r');
    bool punct = (c > 0x20 && c < 0x30) || (c > 0x39 && c < 0x41) ||
                 (c > 0x5A && c < 0x61) || (c > 0x7A && c < 0x7F);
    bool cntrl = c < 0x20 || c == 0x7F;
    bool xdigit = digit || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    bool blank = c == ' ' || c == '\t';
    bool alpha = upper || lower;

    REQUIRE(!!isalnum(c) == (alpha || digit));
    REQUIRE(!!isalpha(c) == alpha);
    REQUIRE(!!isblank(c) == blank);
    REQUIRE(!!iscntrl(c) == cntrl);
    REQUIRE(!!isdigit(c) == digit);
    REQUIRE(!!isgraph(c) == (alpha || digit || punct));
    REQUIRE(!!islower(c) == lower);
    REQUIRE(!!isprint(c) == (alpha || digit || punct || c == ' '));
    REQUIRE(!!ispunct(c) == punct);
    REQUIRE(!!isspace(c) == space);
    REQUIRE(!!isupper(c) == upper);
    REQUIRE(!!isxdigit(c) == xdigit);
  }

  // C11 correction: '\t' is blank (wine's native table lacks the bit)
  REQUIRE(isblank('\t') != 0);
  REQUIRE(isblank(' ') != 0);
  REQUIRE(isblank('a') == 0);
  REQUIRE(isblank(0xA0) == 0);
}

TEST_CASE("ctype narrow range edges")
{
  // Out-of-table inputs are 0, EOF included (wine anchor)
  for (int c : {-1, -2, 256, 0x1000, 0x4F60}) {
    REQUIRE(isalpha(c) == 0);
    REQUIRE(isalnum(c) == 0);
    REQUIRE(isxdigit(c) == 0);
    REQUIRE(isspace(c) == 0);
    REQUIRE(iscntrl(c) == 0);
    REQUIRE(isleadbyte(c) == 0);
  }
}

TEST_CASE("_isctype")
{
  REQUIRE(_isctype('A', _UPPER) != 0);
  REQUIRE(_isctype('A', _HEX) != 0);
  REQUIRE(_isctype('a', _LOWER) != 0);
  REQUIRE(_isctype('0', _DIGIT) != 0);
  REQUIRE(_isctype(' ', _BLANK) != 0);
  REQUIRE(_isctype('\t', _BLANK) != 0);

  // wine anchors: out-of-range -> 0 (no low-byte fallback)
  REQUIRE(_isctype(-1, _UPPER) == 0);
  REQUIRE(_isctype(256, _UPPER) == 0);
  REQUIRE(_isctype(0x4F60, _UPPER) == 0);
  REQUIRE(_isctype(0x4F60, _ALPHA) == 0);

  // >= 0x80 never classifies; composite _ALPHA stays ASCII-only
  REQUIRE(_isctype(0xE9, _LOWER) == 0);
  REQUIRE(_isctype(0xE9, _ALPHA) == 0);

  // _l variant ignores the locale argument
  REQUIRE(_isctype_l('A', _UPPER, nullptr) != 0);
  REQUIRE(_isctype_l(0xE9, _LOWER, nullptr) == 0);
}

TEST_CASE("__iscsym and __iscsymf")
{
  REQUIRE(__iscsym('_') != 0);
  REQUIRE(__iscsym('1') != 0);
  REQUIRE(__iscsym('A') != 0);
  REQUIRE(__iscsym('z') != 0);
  REQUIRE(__iscsym('+') == 0);
  REQUIRE(__iscsym(0xE9) == 0);

  REQUIRE(__iscsymf('_') != 0);
  REQUIRE(__iscsymf('1') == 0);
  REQUIRE(__iscsymf('A') != 0);
  REQUIRE(__iscsymf(0xE9) == 0);
}

TEST_CASE("isleadbyte always false")
{
  for (int c : {129, 130, 161, 233, (int)'A', 0, 127, 256})
    REQUIRE(isleadbyte(c) == 0);
  REQUIRE(_isleadbyte_l(0x81, nullptr) == 0);
}

TEST_CASE("ctype pollution adversarial")
{
  // Native state flips: CP1252-ish locale makes 0xE9 alpha, Turkish
  // folds I/i apart.  The overlay predicates stay fixed ASCII.
  wchar_t *r1 = _wsetlocale(LC_CTYPE, L"french");
  if (r1)
    INFO("french locale accepted");
  REQUIRE(isalpha(0xE9) == 0);
  REQUIRE(isalnum(0xE9) == 0);
  REQUIRE(_isctype(0xE9, _ALPHA) == 0);
  REQUIRE(isprint(0xE9) == 0);
  REQUIRE(_stricmp("I", "i") == 0);

  wchar_t *r2 = _wsetlocale(LC_CTYPE, L"turkish");
  if (r2)
    INFO("turkish locale accepted");
  REQUIRE(isalpha('I') != 0);
  REQUIRE(islower('I') == 0);
  REQUIRE(_stricmp("I", "i") == 0);

  _wsetlocale(LC_CTYPE, L"C");
}

TEST_CASE("_is*_l spot checks")
{
  REQUIRE(_isalpha_l('A', nullptr) != 0);
  REQUIRE(_isalpha_l(0xE9, nullptr) == 0);
  REQUIRE(_isblank_l('\t', nullptr) != 0);
  REQUIRE(_isblank_l('a', nullptr) == 0);
  REQUIRE(_isxdigit_l('f', nullptr) != 0);
  REQUIRE(_isxdigit_l('g', nullptr) == 0);
}
