#include <cstdio>
#include <stdexcept>

extern "C" __declspec(dllimport) int sum_to_n(int);
extern "C" __declspec(dllimport) const char* library_id();
extern "C" __declspec(dllimport) void throw_across_boundary(int);

int main() {
  printf("id = %s\n", library_id());
  printf("sum = %d\n", sum_to_n(100));

  try {
    throw_across_boundary(1);
  } catch (const std::runtime_error& e) {
    printf("caught: %s\n", e.what());
  }
  return 0;
}
