#include <errno.h>

#include <nocrt/errno.h>

namespace u8crt
{
  extern "C" int *__errno_location()
  {
    return libc::__errno_location();
  }
} // namespace u8crt
