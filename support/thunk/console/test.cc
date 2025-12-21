#include <fcntl.h>
#include <io.h>
#include <stdio.h>

int main()
{
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);

  puts("你好");
  printf("%s\n", "你好");
  return 0;
}
