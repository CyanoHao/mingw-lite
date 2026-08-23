#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"
#include "../mingw/msvcrt.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk::ucrt
{

  int __cdecl _fputc_nolock_internal(int const c,
                                     FILE *const public_stream,
                                     __crt_cached_ptd_host &ptd)
  {
    console *con = ptd.get_console();
    if (!con)
    {
      ptd.get_errno().set(EBADF);
      return EOF;
    }

    int ret = con->putc_nolock(static_cast<unsigned char>(c));

    if (ret == EOF)
    {
      ptd.get_errno().set(errno);
      return EOF;
    }

    return ret;
  }

  int __cdecl _fputc_nolock(int const c, FILE *const public_stream)
  {
    if (!console::is_console(public_stream))
      return ms::_fputc_nolock(c, public_stream);

    __crt_cached_ptd_host ptd;
    return _fputc_nolock_internal(c, public_stream, ptd);
  }

  int __cdecl
  _fputc_internal(int const c, FILE *const stream, __crt_cached_ptd_host &ptd)
  {
    _UCRT_VALIDATE_RETURN(ptd, stream != nullptr, EINVAL, EOF);

    return __acrt_lock_stream_and_call(
        stream, [&] { return _fputc_nolock_internal(c, stream, ptd); });
  }

  int __cdecl fputc(int const c, FILE *const stream)
  {
    if (!console::is_console(stream))
      return __ms_fputc(c, stream);

    __crt_cached_ptd_host ptd;
    return _fputc_internal(c, stream, ptd);
  }

} // namespace mingw_thunk::ucrt
