#include <stdlib.h>

#include <nocrt/stdlib.h>

namespace u8crt
{
  extern "C" void free(void *ptr)
  {
    return libc::free(ptr);
  }
} // namespace u8crt
