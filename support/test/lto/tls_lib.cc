#include <atomic>
#include <cstdint>
#include <cstdio>

thread_local int64_t tls_counter = 0;
std::atomic<int64_t> total{0};

void increment_tls(int n) {
  for (int i = 0; i < n; ++i) {
    tls_counter++;
    total++;
  }
  printf("local = %lld\n", (long long)tls_counter);
}
