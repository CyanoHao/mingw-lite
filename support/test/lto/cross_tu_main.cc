#include <cstdint>
#include <cstdio>

int64_t sum_cpp(int);

int main() {
  printf("sum = %lld\n", (long long)sum_cpp(100));
  return 0;
}
