#include "FormatMessageW.h"

#include <catch_amalgamated.hpp>

#include <windows.h>

#include <stdarg.h>

TEST_CASE("FormatMessageW")
{
  SECTION("FORMAT_MESSAGE_FROM_SYSTEM with allocated buffer")
  {
    LPWSTR buffer = nullptr;
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr,
        ERROR_FILE_NOT_FOUND,
        0,
        (LPWSTR)&buffer,
        0,
        nullptr);

    REQUIRE(result > 0);
    REQUIRE(buffer != nullptr);

    REQUIRE(wcslen(buffer) > 0);

    LocalFree(buffer);
  }

  SECTION("FORMAT_MESSAGE_FROM_SYSTEM with fixed buffer")
  {
    wchar_t buffer[256];
    DWORD result =
        mingw_thunk::f::win9x_FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                                             nullptr,
                                             ERROR_FILE_NOT_FOUND,
                                             0,
                                             buffer,
                                             sizeof(buffer) / sizeof(wchar_t),
                                             nullptr);

    REQUIRE(result > 0);
    REQUIRE(wcslen(buffer) > 0);
  }

  SECTION("FORMAT_MESSAGE_FROM_SYSTEM with insufficient fixed buffer")
  {
    wchar_t buffer[10];
    DWORD result =
        mingw_thunk::f::win9x_FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                                             nullptr,
                                             ERROR_FILE_NOT_FOUND,
                                             0,
                                             buffer,
                                             sizeof(buffer) / sizeof(wchar_t),
                                             nullptr);

    REQUIRE(result == 0);
    REQUIRE(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  }

  SECTION("FORMAT_MESSAGE_FROM_STRING with fixed buffer")
  {
    ULONG_PTR inserts[1] = {(ULONG_PTR) "message"};
    wchar_t buffer[256];
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        L"Test message: %1",
        0,
        0,
        buffer,
        sizeof(buffer) / sizeof(wchar_t),
        (va_list *)inserts);

    REQUIRE(result > 0);
    REQUIRE(wcscmp(buffer, L"Test message: message") == 0);
  }

  SECTION("FORMAT_MESSAGE_FROM_STRING with allocated buffer")
  {
    LPWSTR buffer = nullptr;
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        L"Simple test message",
        0,
        0,
        (LPWSTR)&buffer,
        0,
        nullptr);

    REQUIRE(result > 0);
    REQUIRE(buffer != nullptr);
    REQUIRE(wcslen(buffer) > 0);

    LocalFree(buffer);
  }

  SECTION("FORMAT_MESSAGE_FROM_STRING with inserts and allocated buffer")
  {
    ULONG_PTR numbers[1] = {42};
    LPWSTR buffer = nullptr;
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
        L"Error code: %1!d!",
        0,
        0,
        (LPWSTR)&buffer,
        0,
        (va_list *)numbers);

    REQUIRE(result > 0);
    REQUIRE(buffer != nullptr);
    REQUIRE(wcscmp(buffer, L"Error code: 42") == 0);

    LocalFree(buffer);
  }

  SECTION("FORMAT_MESSAGE_ALLOCATE_BUFFER with ERROR_SUCCESS")
  {
    LPWSTR buffer = nullptr;
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr,
        ERROR_SUCCESS,
        0,
        (LPWSTR)&buffer,
        0,
        nullptr);

    REQUIRE(result > 0);
    REQUIRE(buffer != nullptr);

    LocalFree(buffer);
  }

  SECTION("Long message with system allocation")
  {
    LPWSTR buffer = nullptr;
    DWORD result = mingw_thunk::f::win9x_FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        L"This is a very long test message that exceeds MAX_PATH characters: "
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890"
        L"123456789012345678901234567890123456789012345678901234567890",
        0,
        0,
        (LPWSTR)&buffer,
        0,
        nullptr);

    REQUIRE(result > 0);
    REQUIRE(buffer != nullptr);
    REQUIRE(wcslen(buffer) > 259);

    LocalFree(buffer);
  }
}
