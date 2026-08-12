#include <stdexcept>
#include <cstdio>

void do_throw(int);
void rethrow_inner(int);

int main() {
  int caught = 0;

  try {
    do_throw(0);
  } catch (...) {
    caught++;
  }

  try {
    do_throw(-1);
  } catch (const std::invalid_argument& e) {
    printf("invalid: %s\n", e.what());
    caught++;
  }

  try {
    rethrow_inner(42);
  } catch (const std::runtime_error& e) {
    printf("runtime: %s\n", e.what());
    caught++;
  }

  printf("caught = %d\n", caught);
  return 0;
}
