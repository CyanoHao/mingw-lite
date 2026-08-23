#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk::ucrt
{

  // Writes an array of items to a stream.  Console streams take the
  // coupled stream lock and push the bytes nolock through the port
  // console pool; the return keeps the complete-items contract.
  size_t __cdecl
  fwrite(void const *const ptr, size_t const size, size_t const count, FILE *const stream)
  {
    if (size == 0 || count == 0)
      return 0;

    if (!console::is_console(stream))
      return __ms_fwrite(ptr, size, count, stream);

    size_t const total = size * count;

    __crt_cached_ptd_host ptd;

    return __acrt_lock_stream_and_call(
        stream,
        ptd,
        [&](console *con) -> size_t
        {
          if (!con)
          {
            errno = EBADF;
            return 0;
          }

          return con->write_nolock(ptr, total) / size;
        });
  }

} // namespace mingw_thunk::ucrt
