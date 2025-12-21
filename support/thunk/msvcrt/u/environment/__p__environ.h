#include <thunk/_common.h>

#include <nostl/string.h>
#include <nostl/vector.h>

namespace mingw_thunk
{
  namespace internal
  {
    extern char **u8_environ;
    extern size_t u8_environ_size;
    extern bool u8_environ_lock;
  } // namespace internal
} // namespace mingw_thunk
