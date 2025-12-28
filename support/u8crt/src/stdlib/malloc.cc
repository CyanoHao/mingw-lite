#include <stdlib.h>

#include <nocrt/stdlib.h>

namespace u8crt
{
  extern "C" void *malloc(size_t size)
  {
    return libc::malloc(size);
  }
} // namespace u8crt
