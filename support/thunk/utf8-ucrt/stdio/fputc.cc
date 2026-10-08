#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"
#include "../mingw/msvcrt.h"
#include "../mingw/thunk.h"

namespace mingw_thunk::ucrt
{

  int __cdecl _fputc_nolock_internal(int const c,
                                     FILE *const public_stream,
                                     __crt_cached_ptd_host &ptd)
  {
    int const fd = _fileno(public_stream);

    if (!is_console(fd))
    {
      int ret = ms::_fputc_nolock(c, public_stream);
      if (ret == EOF)
        ptd.get_errno().set(errno);
      return ret;
    }

    console *con = console::get(fd, true);
    if (!con)
    {
      return EOF;
    }

    auto guard = con->acquire_guard();
    int ret = con->put_nolock(c);

    if (ret != EOF)
      con->flush_stdout_or_stderr_nolock();

    return ret;
  }

} // namespace mingw_thunk::ucrt
