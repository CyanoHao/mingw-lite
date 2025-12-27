#include <string.h>

#include <nocrt/string.h>

namespace u8crt
{
  extern "C" int memcmp(const void *lhs, const void *rhs, size_t count)
  {
    return libc::memcmp(lhs, rhs, count);
  }
} // namespace u8crt
