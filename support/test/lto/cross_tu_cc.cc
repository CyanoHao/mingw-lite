#include <cstdint>

extern "C" int sum_c(int);

int64_t sum_cpp(int n) {
  int64_t s = 0;
  for (int i = 1; i <= n; ++i)
    s += sum_c(i);
  return s;
}
