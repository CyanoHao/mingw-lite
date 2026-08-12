#include <stdexcept>
#include <string>

void do_throw(int code) {
  if (code == 0)
    return;
  if (code < 0)
    throw std::invalid_argument("negative code");
  throw std::runtime_error("code " + std::to_string(code));
}

void rethrow_inner(int code) {
  try {
    do_throw(code);
  } catch (const std::exception&) {
    throw;
  }
}
