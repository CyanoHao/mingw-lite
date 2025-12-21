#include <stdio.h>

#include <stringapiset.h>

int main()
{
  const wchar_t *w_str = L"‘hello’";
  char str[1024] = {0};
  WideCharToMultiByte(CP_UTF8, 0, w_str, -1, str, 1024, NULL, NULL);

  puts(str);
  printf("%s\n", str);
  return 0;
}
