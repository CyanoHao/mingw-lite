#include <stdlib.h>

#include <nocrt/__stdlib/malloc.h>

namespace u8crt
{
  extern "C" void *malloc(size_t size)
  {
    return libc::malloc(size);
  }
} // namespace u8crt
