#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <mbctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

  // M10 faces not declared by the mingw headers
  extern "C" unsigned int __cdecl ___lc_collate_cp_func(void);
  extern "C" wchar_t **__cdecl ___lc_locale_name_func(void);
  extern "C" int __cdecl ___mb_cur_max_l_func(_locale_t locale);
  // (__pctype_func/__pwctype_func/_configthreadlocale/_getmbcp/_setmbcp/
  //  ___mb_cur_max_func are declared by the mingw headers)
// native pollution vector (def-backed alias, api-ms-win-crt-locale def)
extern "C" __attribute__((dllimport)) int __cdecl
__ms__setmbcp(int code_page);

TEST_CASE("locale sentinel identity")
{
  // D5/D10: one process-wide sentinel, forever the same pointer
  _locale_t a = _create_locale(LC_ALL, "C");
  _locale_t b = _create_locale(LC_ALL, "Chinese_China.936");
  _locale_t c = _get_current_locale();
  REQUIRE(a != nullptr);
  REQUIRE(b == a);
  REQUIRE(c == a);

  _free_locale(a);
  _free_locale(nullptr);
  REQUIRE(_get_current_locale() == a); // free is a no-op

  _locale_t w = _wcreate_locale(LC_ALL, L"C");
  REQUIRE(w == a);
}

TEST_CASE("_create_locale validation")
{
  // wine anchors (probe B / r16): null name and category outside
  // [0,5] -> NULL.  Bogus names and "" return the sentinel here
  // (native: NULL / user default — D10 lenient-sentinel divergence).
  REQUIRE(_create_locale(LC_ALL, nullptr) == nullptr);
  REQUIRE(_create_locale(-1, "C") == nullptr);
  REQUIRE(_create_locale(6, "C") == nullptr);
  REQUIRE(_create_locale(99, "C") == nullptr);

  _locale_t sentinel = _get_current_locale();
  REQUIRE(_create_locale(LC_ALL, "zzz_ZZ") == sentinel);
  REQUIRE(_create_locale(LC_ALL, "") == sentinel);
  REQUIRE(_wcreate_locale(LC_ALL, L"zzz_ZZ") == sentinel);
  REQUIRE(_wcreate_locale(LC_ALL, nullptr) == nullptr);
  REQUIRE(_wcreate_locale(-1, L"C") == nullptr);
}

TEST_CASE("_wsetlocale self-report")
{
  // probe B: native null-query -> "C", rejects L"C.UTF-8"; we always
  // self-report the single overlay locale (documented divergence)
  errno = 0;
  REQUIRE(wcscmp(_wsetlocale(LC_ALL, nullptr), L"C.UTF-8") == 0);
  REQUIRE(wcscmp(_wsetlocale(LC_ALL, L"C"), L"C.UTF-8") == 0);
  REQUIRE(wcscmp(_wsetlocale(LC_ALL, L"chinese"), L"C.UTF-8") == 0);
  REQUIRE(errno == 0);
  REQUIRE(_wsetlocale(-1, nullptr) == nullptr);
  REQUIRE(_wsetlocale(6, L"C") == nullptr);
}

TEST_CASE("locale self-report internals")
{
  // native C state: collate_cp == 0, six NULL name slots,
  // mb_cur_max == 1 — all self-descriptive divergences (D12)
  REQUIRE(___lc_collate_cp_func() == 65001);
  REQUIRE(___lc_codepage_func() == 65001);

  wchar_t **names = ___lc_locale_name_func();
  REQUIRE(names != nullptr);
  for (int i = 0; i < 6; ++i) {
    REQUIRE(names[i] != nullptr);
    REQUIRE(wcscmp(names[i], L"C.UTF-8") == 0);
  }

  REQUIRE(___mb_cur_max_func() == 4);
  REQUIRE(___mb_cur_max_l_func(nullptr) == 4);
  REQUIRE(___mb_cur_max_l_func(_get_current_locale()) == 4);
}

TEST_CASE("ctype table exports")
{
  // __pctype_func: the M8 narrow table — high half must stay zero
  // (UTF-8 bytes are not characters)
  const unsigned short *pc = __pctype_func();
  REQUIRE(pc[9] == 0x0068);
  REQUIRE(pc[0x20] == 0x0048);
  REQUIRE(pc['A'] == 0x0081);
  REQUIRE(pc['a'] == 0x0082);
  REQUIRE(pc['0'] == 0x0084);
  for (int c = 0x80; c < 0x100; ++c)
    REQUIRE(pc[c] == 0);

  // __pwctype_func: the wine-anchored wide table (probe "PWCT") —
  // Unicode categories in the high half, '\t' _BLANK corrected
  const unsigned short *wc = __pwctype_func();
  REQUIRE(wc[9] == 0x0068);
  REQUIRE(wc[0x20] == 0x0048);
  REQUIRE(wc['A'] == 0x0181);
  REQUIRE(wc['a'] == 0x0182);
  REQUIRE(wc['0'] == 0x0084);
  REQUIRE(wc[0x7F] == 0x0020);
  REQUIRE(wc[0x85] == 0x0028); // NEL: control|space
  REQUIRE(wc[0xA0] == 0x0008); // NBSP: space only
  REQUIRE(wc[0xE9] == 0x0102); // é: alpha|lower
  REQUIRE(wc[0xFF] == 0x0102);
  REQUIRE(wc[0xD7] == 0x0010); // ×: punct
  REQUIRE(wc[0xF7] == 0x0010); // ÷: punct
  REQUIRE(wc[0xDF] == 0x0102); // ß: alpha|lower
  // the wide table really differs from the narrow one
  REQUIRE(wc['A'] != pc['A']);
  REQUIRE(wc['0'] == pc['0']);
}

TEST_CASE("_configthreadlocale wine oracle")
{
  // D11 machine — the probe sequences, in order (process-fresh state);
  // errno is never touched (probe G/r10: 0 stays 0)
  errno = 0;
  REQUIRE(_configthreadlocale(0) == 2);
  REQUIRE(_configthreadlocale(1) == 2);
  REQUIRE(_configthreadlocale(0) == 1);
  REQUIRE(_configthreadlocale(2) == 1);
  REQUIRE(_configthreadlocale(0) == 2);
  REQUIRE(_configthreadlocale(-1) == -1);
  REQUIRE(_configthreadlocale(0) == 2);
  REQUIRE(errno == 0);

  // discriminating sequences (state after the block above: P = 0)
  REQUIRE(_configthreadlocale(1) == 2);
  REQUIRE(_configthreadlocale(2) == 1);
  REQUIRE(_configthreadlocale(0) == 2);
  REQUIRE(_configthreadlocale(0) == 2);

  // invalid options never touch the state
  REQUIRE(_configthreadlocale(42) == -1);
  REQUIRE(_configthreadlocale(-3) == -1);
  REQUIRE(_configthreadlocale(0) == 2);
}

TEST_CASE("_setmbcp _getmbcp D9")
{
  errno = 0;
  REQUIRE(_getmbcp() == 65001);

  // succeeds but has no effect (native flips its DBCS tables — probe F)
  REQUIRE(_setmbcp(65001) == 0);
  REQUIRE(_setmbcp(936) == 0);
  REQUIRE(_setmbcp(_MB_CP_ANSI) == 0);
  REQUIRE(_setmbcp(_MB_CP_OEM) == 0);
  REQUIRE(_setmbcp(_MB_CP_SBCS) == 0);
  REQUIRE(_getmbcp() == 65001);
  REQUIRE(errno == 0);

  // invalid: native shape -1 + EINVAL
  errno = 0;
  REQUIRE(_setmbcp(42) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("native mbcp pollution does not leak")
{
  // R1 reversal setup: native DBCS state driven to CP936 behind our
  // back — the overlay's self-report and tables must not move
  REQUIRE(__ms__setmbcp(936) == 0);

  REQUIRE(_getmbcp() == 65001);
  REQUIRE(__pwctype_func()[0xD7] == 0x0010);
  REQUIRE(__pctype_func()[0x80] == 0);

  // restore the native state for the other tests
  __ms__setmbcp(65001);
}
