#include <cstdio>
#include <string>

#include <nowide/convert.hpp>

int main() {
  {
    const char *original = "Hello, world!";
    const wchar_t *expected = L"Hello, world!";

    std::wstring result = nowide::widen(original);
    if (result != expected)
      return 1;

    std::string round_trip = nowide::narrow(result);
    if (round_trip != original)
      return 1;
  }

  {
    const char *original = "你好，世界！";
    const wchar_t *expected = L"你好，世界！";

    std::wstring result = nowide::widen(original);
    if (result != expected)
      return 1;

    std::string round_trip = nowide::narrow(result);
    if (round_trip != original)
      return 1;
  }

  std::printf("ok\n");
  return 0;
}
