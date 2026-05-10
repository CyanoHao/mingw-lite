#include <cstdio>
#include <cstring>
#include <nowide/fstream.hpp>

int main() {
  const char *name = "test__nowide__fstream__‘hello’.txt";
  const char *content = "Hello, world!";

  {
    nowide::ofstream ofs(name);
    if (!ofs.is_open())
      return 1;
    ofs << content << '\n';
  }

  {
    nowide::ifstream ifs(name);
    if (!ifs.is_open())
      return 1;
    char buf[256];
    if (!ifs.getline(buf, sizeof(buf)))
      return 1;

    if (std::strcmp(buf, content) != 0)
      return 1;
  }

  if (nowide::remove(name) != 0)
    return 1;

  std::printf("%s\n", content);
  return 0;
}
