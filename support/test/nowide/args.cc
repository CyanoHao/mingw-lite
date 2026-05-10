#include <cstdio>

#include <nowide/args.hpp>

int main(int argc, char *argv[]) {
  nowide::args _(argc, argv);

  for (int i = 1; i < argc; i++) {
    std::printf("argv[%d] = %s\n", i, argv[i]);
  }
  return 0;
}
