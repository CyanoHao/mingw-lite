#include <cstdio>
#include <cstring>
#include <nowide/cstdio.hpp>

int main() {
  const char *name = "test__nowide__fopen__‘hello’.txt";
  const char *content = "Hello, world!\n";

  FILE *f = nowide::fopen(name, "w");
  if (!f)
    return 1;
  std::fputs(content, f);
  std::fclose(f);

  char buf[256];
  f = nowide::fopen(name, "r");
  if (!f)
    return 1;
  if (!std::fgets(buf, sizeof(buf), f))
    return 1;
  std::fclose(f);

  if (std::strcmp(buf, content) != 0)
    return 1;

  if (nowide::remove(name) != 0)
    return 1;

  std::printf("%s", content);
  return 0;
}
