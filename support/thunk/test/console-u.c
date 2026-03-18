#include <stdio.h>

#include <windows.h>

void puts_w(const wchar_t *s)
{
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  size_t len = wcslen(s);
  DWORD written;
  WriteConsoleW(hStdout, L"[note] ", 7, &written, NULL);
  WriteConsoleW(hStdout, s, len, &written, NULL);
  WriteConsoleW(hStdout, L"\n", 1, &written, NULL);
}

int main()
{
  puts_w(L"printf short string: 天地玄黄");
  printf("%s%s\n", "天地", "玄黄");
  return 0;
}
