#include <atomic>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

void increment_tls(int);
extern std::atomic<int64_t> total;

int main() {
  constexpr int N = 4;
  constexpr int ITERS = 1000;

  std::vector<std::thread> threads;
  threads.reserve(N);
  for (int i = 0; i < N; ++i)
    threads.emplace_back([ITERS] { increment_tls(ITERS); });
  for (auto& t : threads)
    t.join();

  printf("total = %lld\n", (long long)total.load());
  return 0;
}
