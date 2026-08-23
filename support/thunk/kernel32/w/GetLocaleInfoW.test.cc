#include "GetLocaleInfoW.h"

#include <catch_amalgamated.hpp>

#include <windows.h>

TEST_CASE("GetLocaleInfoW")
{
  wchar_t buffer[64];

  SECTION("english text")
  {
    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_SENGLANGUAGE, buffer, 64);
    REQUIRE(length == 8);
    REQUIRE(wcscmp(buffer, L"English") == 0);

    const int dry = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_SENGLANGUAGE, NULL, 0);
    REQUIRE(dry == 8);
  }

  SECTION("codepage digits")
  {
    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_IDEFAULTANSICODEPAGE, buffer, 64);
    REQUIRE(length == 5);
    REQUIRE(wcscmp(buffer, L"1252") == 0);
  }

  SECTION("cjk locale roundtrip")
  {
    char a_buffer[64];
    const int a_length =
        GetLocaleInfoA(0x0804, LOCALE_SMONTHNAME1, a_buffer, 64);
    REQUIRE(a_length > 0);

    char cp_buffer[16];
    const int cp_length = GetLocaleInfoA(
        0x0804, LOCALE_IDEFAULTANSICODEPAGE, cp_buffer, 16);
    REQUIRE(cp_length > 0);

    UINT cp = 0;
    for (const char *p = cp_buffer; *p >= '0' && *p <= '9'; ++p)
      cp = cp * 10 + (*p - '0');
    REQUIRE(cp != 0);

    const int expected_length =
        MultiByteToWideChar(cp, 0, a_buffer, a_length, NULL, 0);
    REQUIRE(expected_length > 0);

    wchar_t expected[64];
    REQUIRE(MultiByteToWideChar(
                cp, 0, a_buffer, a_length, expected, expected_length) ==
            expected_length);

    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0804, LOCALE_SMONTHNAME1, buffer, 64);
    REQUIRE(length == expected_length);
    REQUIRE(wcscmp(buffer, expected) == 0);

    const int dry = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0804, LOCALE_SMONTHNAME1, NULL, 0);
    REQUIRE(dry == expected_length);
  }

  SECTION("return number")
  {
    const int dry = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, NULL, 0);
    REQUIRE(dry == 2);

    DWORD value = 0;
    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (wchar_t *)&value, 2);
    REQUIRE(length == 2);
    REQUIRE(value == 0x0409);
  }

  SECTION("font signature")
  {
    LOCALESIGNATURE signature = {};
    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_FONTSIGNATURE, (wchar_t *)&signature, 16);
    REQUIRE(length == 16);
    REQUIRE(signature.lsUsb[0] != 0);
  }

  SECTION("insufficient buffer")
  {
    SetLastError(0);
    wchar_t small[8];
    const int length = mingw_thunk::f::win9x_GetLocaleInfoW(
        0x0409, LOCALE_SENGLANGUAGE, small, 7);
    REQUIRE(length == 0);
    REQUIRE(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  }

  SECTION("invalid parameters")
  {
    SetLastError(0);
    REQUIRE(mingw_thunk::f::win9x_GetLocaleInfoW(
                0x0409, LOCALE_SENGLANGUAGE, buffer, -1) == 0);
    REQUIRE(GetLastError() == ERROR_INVALID_PARAMETER);

    SetLastError(0);
    REQUIRE(mingw_thunk::f::win9x_GetLocaleInfoW(
                0x0409, LOCALE_SENGLANGUAGE, NULL, 8) == 0);
    REQUIRE(GetLastError() == ERROR_INVALID_PARAMETER);
  }
}
