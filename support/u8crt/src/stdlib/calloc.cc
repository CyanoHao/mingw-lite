#include <errno.h>
#include <stdlib.h>

#include <nocrt/stdlib.h>

namespace u8crt
{
  extern "C" void *calloc(size_t nmemb, size_t size)
  {
    if (__SIZE_MAX__ / size < nmemb) {
      errno = ENOMEM;
      return nullptr;
    }

    return libc::malloc(nmemb * size);
  }
} // namespace u8crt
