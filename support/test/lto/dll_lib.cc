#include <stdexcept>
#include <string>

extern "C" __declspec(dllexport) int sum_to_n(int n) {
  int s = 0;
  for (int i = 1; i <= n; ++i)
    s += i;
  return s;
}

extern "C" __declspec(dllexport) const char* library_id() {
  return "lto-dll-v1";
}

extern "C" __declspec(dllexport) void throw_across_boundary(int code) {
  if (code != 0)
    throw std::runtime_error("from-dll");
}
