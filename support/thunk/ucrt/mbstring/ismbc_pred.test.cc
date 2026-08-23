#include <catch_amalgamated.hpp>

#include <locale.h>

#include "mbs_test_matrix.h"

// _ismbc* is not declared by any mingw header, and the _ismbb* family
// is only reachable through mbctype.h/mbstring.h, so both are declared
// here directly.  i686 hides some of these from the headers entirely;
// declaring them ourselves is the M12 finding applied consistently.
extern "C"
{
  int __cdecl _ismbcalnum(unsigned int c);
  int __cdecl _ismbcalpha(unsigned int c);
  int __cdecl _ismbcblank(unsigned int c);
  int __cdecl _ismbcdigit(unsigned int c);
  int __cdecl _ismbcgraph(unsigned int c);
  int __cdecl _ismbchira(unsigned int c);
  int __cdecl _ismbckata(unsigned int c);
  int __cdecl _ismbcl0(unsigned int c);
  int __cdecl _ismbcl1(unsigned int c);
  int __cdecl _ismbcl2(unsigned int c);
  int __cdecl _ismbclegal(unsigned int c);
  int __cdecl _ismbclower(unsigned int c);
  int __cdecl _ismbcprint(unsigned int c);
  int __cdecl _ismbcpunct(unsigned int c);
  int __cdecl _ismbcspace(unsigned int c);
  int __cdecl _ismbcsymbol(unsigned int c);
  int __cdecl _ismbcupper(unsigned int c);

  int __cdecl _ismbcalnum_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbchira_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbcl1_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbclegal_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbcupper_l(unsigned int c, _locale_t locale);
  int __cdecl _ismbcspace_l(unsigned int c, _locale_t locale);

  int __cdecl __ms__setmbcp(int code_page);
} // extern "C"

namespace
{
  // Every _ismbc* answer must be exactly 0 or 1 (plan-3 D8e): the
  // family is a predicate, not a ctype-mask query like the narrow is*.
  void check_binary(const char *what, int got)
  {
    INFO(what << " returned " << got);
    REQUIRE((got == 0 || got == 1));
  }
} // namespace

TEST_CASE("_ismbc* block anchors")
{
  // Hiragana U+3042 / katakana U+30A2 / CJK U+4E00: the three blocks the
  // reference family exists for.
  REQUIRE(_ismbchira(0x3042) == 1);
  REQUIRE(_ismbckata(0x30A2) == 1);
  REQUIRE(_ismbchira(0x30A2) == 0);
  REQUIRE(_ismbckata(0x3042) == 0);
  REQUIRE(_ismbcl1(0x4E00) == 1);
  REQUIRE(_ismbcl0(0x4E00) == 0);
  REQUIRE(_ismbcl2(0x4E00) == 0);

  // Alpha/alnum reach CJK through the composite _ALPHA bit (plan-3 D8k),
  // which is why the wide CT_CTYPE1 word is consulted at all.
  REQUIRE(_ismbcalpha(0x4E00) == 1);
  REQUIRE(_ismbcalnum(0x4E00) == 1);

  // ASCII answers the ordinary way.
  REQUIRE(_ismbcalpha('A') == 1);
  REQUIRE(_ismbcalpha('a') == 1);
  REQUIRE(_ismbcalpha('0') == 0);
  REQUIRE(_ismbcalnum('0') == 1);
  REQUIRE(_ismbcdigit('0') == 1);
  REQUIRE(_ismbcdigit('a') == 0);
  REQUIRE(_ismbcupper('A') == 1);
  REQUIRE(_ismbcupper('a') == 0);
  REQUIRE(_ismbclower('a') == 1);
  REQUIRE(_ismbclower('A') == 0);
  REQUIRE(_ismbcspace(' ') == 1);
  REQUIRE(_ismbcspace('a') == 0);
  REQUIRE(_ismbcblank(' ') == 1);
  REQUIRE(_ismbcblank('\t') == 1);
  REQUIRE(_ismbcblank('\n') == 0);
  REQUIRE(_ismbcpunct('!') == 1);
  REQUIRE(_ismbcpunct('a') == 0);
  REQUIRE(_ismbcprint('\n') == 0);
  REQUIRE(_ismbcprint('a') == 1);
  REQUIRE(_ismbcgraph(' ') == 0);
  REQUIRE(_ismbcgraph('a') == 1);
  REQUIRE(_ismbcgraph('\n') == 0);
}

TEST_CASE("_ismbc* shared boundary matrix")
{
  using namespace mbsmatrix;

  for (const CpSample &s : kCp) {
    INFO("U+" << std::hex << s.cp << " (" << s.what << ")");

    // D8f: level 0 is [0x3000,0x33FF].
    REQUIRE(_ismbcl0(s.cp) == (s.cp >= 0x3000 && s.cp <= 0x33FF ? 1 : 0));
    // D8f: level 1 is [0x4E00,0x9FA5].
    REQUIRE(_ismbcl1(s.cp) == (s.cp >= 0x4E00 && s.cp <= 0x9FA5 ? 1 : 0));
    // D8f: level 2 is ext-A plus the two unified tail ranges.
    REQUIRE(_ismbcl2(s.cp) == ((s.cp >= 0x3400 && s.cp <= 0x4DBF) ||
                                       (s.cp >= 0x9FA6 && s.cp <= 0x9FFF) ||
                                       (s.cp >= 0xF900 && s.cp <= 0xFAFF)
                                   ? 1
                                   : 0));
    // D8h/D7c: kana windows are the ones the conversion faces use.
    REQUIRE(_ismbchira(s.cp) == (s.cp >= 0x3041 && s.cp <= 0x3096 ? 1 : 0));
    REQUIRE(_ismbckata(s.cp) == (s.cp >= 0x30A1 && s.cp <= 0x30F6 ? 1 : 0));
    // D8h: legal == "is a Unicode scalar value".
    const bool scalar = s.cp <= 0x10FFFF && !(s.cp >= 0xD800 && s.cp <= 0xDFFF);
    REQUIRE(_ismbclegal(s.cp) == (scalar ? 1 : 0));

    // D8g: the three JIS X 0208 row 1 ranges.
    const bool sym = (s.cp >= 0x3000 && s.cp <= 0x303F) ||
                     (s.cp >= 0xFF01 && s.cp <= 0xFF60) ||
                     (s.cp >= 0xFFE0 && s.cp <= 0xFFE6);
    REQUIRE(_ismbcsymbol(s.cp) == (sym ? 1 : 0));

    // A lone surrogate or an out-of-range value is never anything.
    if (!scalar) {
      check_binary("llegal", _ismbclegal(s.cp));
      REQUIRE(_ismbcalpha(s.cp) == 0);
      REQUIRE(_ismbcalnum(s.cp) == 0);
      REQUIRE(_ismbchira(s.cp) == 0);
      REQUIRE(_ismbckata(s.cp) == 0);
      REQUIRE(_ismbcl0(s.cp) == 0);
      REQUIRE(_ismbcl1(s.cp) == 0);
      REQUIRE(_ismbcl2(s.cp) == 0);
      REQUIRE(_ismbcsymbol(s.cp) == 0);
    }
  }
}

TEST_CASE("_ismbc* full binary-oracle sweep over the 0..0xFFFF plane")
{
  // The three block predicates are the only ones with a closed-form
  // contract, so they get an exhaustive sweep; the rest are checked
  // against their own definition on the same pass.
  for (unsigned c = 0; c <= 0xFFFF; c++) {
    const bool hira = c >= 0x3041 && c <= 0x3096;
    const bool kata = c >= 0x30A1 && c <= 0x30F6;
    const bool l0 = c >= 0x3000 && c <= 0x33FF;
    const bool l1 = c >= 0x4E00 && c <= 0x9FA5;
    const bool l2 = (c >= 0x3400 && c <= 0x4DBF) ||
                    (c >= 0x9FA6 && c <= 0x9FFF) ||
                    (c >= 0xF900 && c <= 0xFAFF);
    const bool scalar = !(c >= 0xD800 && c <= 0xDFFF);

    INFO("U+" << std::hex << c);
    REQUIRE(_ismbchira(c) == (hira ? 1 : 0));
    REQUIRE(_ismbckata(c) == (kata ? 1 : 0));
    REQUIRE(_ismbcl0(c) == (l0 ? 1 : 0));
    REQUIRE(_ismbcl1(c) == (l1 ? 1 : 0));
    REQUIRE(_ismbcl2(c) == (l2 ? 1 : 0));
    REQUIRE(_ismbclegal(c) == (scalar ? 1 : 0));

    // l0/l1/l2 are documented as disjoint (plan-3 D8f).
    if (l0) {
      REQUIRE(l1 == false);
      REQUIRE(l2 == false);
    }
    if (l1) {
      REQUIRE(l2 == false);
    }
  }
}

TEST_CASE("_ismbc* symbol edges")
{
  REQUIRE(_ismbcsymbol(0x3000) == 1);
  REQUIRE(_ismbcsymbol(0x303F) == 1);
  REQUIRE(_ismbcsymbol(0x3040) == 0);
  REQUIRE(_ismbcsymbol(0xFF00) == 0);
  REQUIRE(_ismbcsymbol(0xFF01) == 1);
  REQUIRE(_ismbcsymbol(0xFF60) == 1);
  REQUIRE(_ismbcsymbol(0xFF61) == 0);
  REQUIRE(_ismbcsymbol(0xFFDF) == 0);
  REQUIRE(_ismbcsymbol(0xFFE0) == 1);
  REQUIRE(_ismbcsymbol(0xFFE6) == 1);
  REQUIRE(_ismbcsymbol(0xFFE7) == 0);
  // U+30FC (the prolonged sound mark) sits in the katakana block but
  // outside the D7c conversion window, so the two families legitimately
  // disagree about it.  Neither of the katakana punctuation forms is a
  // D8g symbol either: that contract is the JIS X 0208 row 1 ranges, and
  // the approximation is documented as such.
  REQUIRE(_ismbckata(0x30FC) == 0);
  REQUIRE(_ismbcsymbol(0x30FB) == 0);
  REQUIRE(_ismbcsymbol(0x30FC) == 0);
  REQUIRE(_ismbcsymbol(0x3040) == 0);
}

TEST_CASE("_ismbc* _l twins ignore the locale")
{
  REQUIRE(_ismbcalnum_l(0x4E00, nullptr) == _ismbcalnum(0x4E00));
  REQUIRE(_ismbchira_l(0x3042, nullptr) == 1);
  REQUIRE(_ismbcl1_l(0x4E00, nullptr) == 1);
  REQUIRE(_ismbclegal_l(0xD800, nullptr) == 0);
  REQUIRE(_ismbclegal_l(0x110000, nullptr) == 0);
  REQUIRE(_ismbcspace_l(' ', nullptr) == 1);
  REQUIRE(_ismbcspace_l('a', nullptr) == 0);
  REQUIRE(_ismbcupper_l('A', nullptr) == 1);

  // A real locale_t must not change the answer either.
  _locale_t loc = _create_locale(LC_ALL, "C");
  REQUIRE(_ismbcalnum_l(0x4E00, loc) == 1);
  REQUIRE(_ismbchira_l(0x3042, loc) == 1);
  _free_locale(loc);
}

TEST_CASE("_ismbc* every face answers 0/1 (plan-3 D8e)")
{
  for (unsigned c : {0u,
                     0x20u,
                     0x41u,
                     0x3042u,
                     0x4E00u,
                     0xD800u,
                     0x10FFFFu,
                     0x110000u,
                     0xFFFFFFFFu}) {
    INFO("U+" << std::hex << c);
    check_binary("alnum", _ismbcalnum(c));
    check_binary("alpha", _ismbcalpha(c));
    check_binary("blank", _ismbcblank(c));
    check_binary("digit", _ismbcdigit(c));
    check_binary("graph", _ismbcgraph(c));
    check_binary("hira", _ismbchira(c));
    check_binary("kata", _ismbckata(c));
    check_binary("l0", _ismbcl0(c));
    check_binary("l1", _ismbcl1(c));
    check_binary("l2", _ismbcl2(c));
    check_binary("llegal", _ismbclegal(c));
    check_binary("lower", _ismbclower(c));
    check_binary("print", _ismbcprint(c));
    check_binary("punct", _ismbcpunct(c));
    check_binary("space", _ismbcspace(c));
    check_binary("symbol", _ismbcsymbol(c));
    check_binary("upper", _ismbcupper(c));
  }
}

TEST_CASE("_ismbc* pollution adversarial")
{
  // plan-3 §5.3 pollution case / D9: asking native to enter a DBCS code
  // page changes nothing here, because the overlay's own _setmbcp refuses
  // every code page but UTF-8.  If this ever starts answering differently
  // the engine is reading native state, which it must never do.
  REQUIRE(__ms__setmbcp(936) == 0);
  REQUIRE(_ismbcalnum(0x4E00) == 1);
  REQUIRE(_ismbchira(0x3042) == 1);
  REQUIRE(_ismbckata(0x30A2) == 1);
  REQUIRE(_ismbclegal(0xD800) == 0);
  // 0xA1 is a Shift-JIS single-byte katakana lead: native calls it alpha
  // under CP932, this engine calls it a control character.
  REQUIRE(_ismbcalnum(0xA1) == 0);
  REQUIRE(_ismbcalpha(0xA1) == 0);
  REQUIRE(_ismbcl1(0xA1) == 0);
  __ms__setmbcp(65001);
}
