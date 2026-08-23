#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"
#include "../mingw/msvcrt.h"

namespace mingw_thunk::ucrt
{

  wint_t __cdecl _fputwc_nolock_internal(wchar_t const c,
                                         FILE *const public_stream,
                                         __crt_cached_ptd_host &ptd)
  {
    if (!ptd.is_console(public_stream))
      return ms::_fputwc_nolock(c, public_stream);

    console *con = ptd.get_console();
    if (!con)
    {
      ptd.get_errno().set(EBADF);
      return WEOF;
    }

    int ret = con->putc_nolock(c);

    if (ret == WEOF)
    {
      ptd.get_errno().set(errno);
      return WEOF;
    }

    return ret;
  }

} // namespace mingw_thunk::ucrt
