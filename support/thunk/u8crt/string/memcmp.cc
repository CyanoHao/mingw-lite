#include <stdlib.h>

#include <nocrt/string.h>

namespace u8crt
{
  extern "C" int memcmp(const void *s1, const void *s2, size_t n)
  {
    return libc::memcmp(s1, s2, n);
  }
} // namespace u8crt
