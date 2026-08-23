#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"
#include "../mingw/thunk.h"

namespace mingw_thunk::ucrt
{

  int __cdecl _fputc_nolock_internal(int const c,
                                     FILE *const public_stream,
                                     __crt_cached_ptd_host &ptd)
  {
    if (!is_console(public_stream))
    {
      int ret = _fputc_nolock(c, public_stream);
      if (ret == EOF)
        ptd.get_errno().set(errno);
      return ret;
    }

    // UTF-8 console path
  }

} // namespace mingw_thunk::ucrt
