#include <catch_amalgamated.hpp>

#include <windows.h>

TEST_CASE("GetLocaleInfoA")
{
  wchar_t w_buffer[64];
  char buffer[64];

  SECTION("locale codepage passthrough")
  {
    const int length = GetLocaleInfoA(0x0409, LOCALE_SENGLANGUAGE, buffer, 64);
    REQUIRE(length == 8);
    REQUIRE(strcmp(buffer, "English") == 0);
  }

  SECTION("USE_CP_ACP ascii text")
  {
    SetLastError(0);
    const int dry = GetLocaleInfoA(
        0x0409, LOCALE_SENGLANGUAGE | LOCALE_USE_CP_ACP, NULL, 0);
    REQUIRE(dry == 8);

    const int length = GetLocaleInfoA(
        0x0409, LOCALE_SENGLANGUAGE | LOCALE_USE_CP_ACP, buffer, 64);
    REQUIRE(length == 8);
    REQUIRE(strcmp(buffer, "English") == 0);
  }

  SECTION("USE_CP_ACP utf-8 conversion")
  {
    const int w_length =
        GetLocaleInfoW(0x0804, LOCALE_SMONTHNAME1, w_buffer, 64);
    REQUIRE(w_length > 0);

    char expected[64];
    const int expected_length = WideCharToMultiByte(
        CP_UTF8, 0, w_buffer, w_length, expected, 64, NULL, NULL);
    REQUIRE(expected_length > 0);

    SetLastError(0);
    const int dry = GetLocaleInfoA(
        0x0804, LOCALE_SMONTHNAME1 | LOCALE_USE_CP_ACP, NULL, 0);
    REQUIRE(dry == expected_length);

    const int length = GetLocaleInfoA(
        0x0804, LOCALE_SMONTHNAME1 | LOCALE_USE_CP_ACP, buffer, 64);
    REQUIRE(length == expected_length);
    REQUIRE(memcmp(buffer, expected, expected_length) == 0);
  }

  SECTION("insufficient buffer")
  {
    SetLastError(0);
    wchar_t small[8];
    const int length = GetLocaleInfoA(
        0x0409, LOCALE_SENGLANGUAGE | LOCALE_USE_CP_ACP, (char *)small, 7);
    REQUIRE(length == 0);
    REQUIRE(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  }

  SECTION("invalid parameters")
  {
    SetLastError(0);
    REQUIRE(GetLocaleInfoA(
                0x0409, LOCALE_SENGLANGUAGE | LOCALE_USE_CP_ACP, buffer, -1) ==
            0);
    REQUIRE(GetLastError() == ERROR_INVALID_PARAMETER);

    SetLastError(0);
    REQUIRE(GetLocaleInfoA(
                0x0409, LOCALE_SENGLANGUAGE | LOCALE_USE_CP_ACP, NULL, 8) == 0);
    REQUIRE(GetLastError() == ERROR_INVALID_PARAMETER);
  }

  SECTION("A-only LCTYPE accepted")
  {
    // deliberately more permissive than the native A contract, which
    // rejects LOCALE_SSHORTTIME with ERROR_INVALID_FLAGS
    const int w_length = GetLocaleInfoW(0x0409, LOCALE_SSHORTTIME, w_buffer, 64);
    REQUIRE(w_length > 0);

    char expected[64];
    const int expected_length = WideCharToMultiByte(
        CP_UTF8, 0, w_buffer, w_length, expected, 64, NULL, NULL);
    REQUIRE(expected_length > 0);

    const int length = GetLocaleInfoA(
        0x0409, LOCALE_SSHORTTIME | LOCALE_USE_CP_ACP, buffer, 64);
    REQUIRE(length == expected_length);
    REQUIRE(memcmp(buffer, expected, expected_length) == 0);
  }
}
