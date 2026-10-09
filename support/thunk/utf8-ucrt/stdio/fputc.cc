#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"
#include "../mingw/msvcrt.h"

namespace mingw_thunk::ucrt
{

  int __cdecl _fputc_nolock_internal(int const c,
                                     FILE *const public_stream,
                                     __crt_cached_ptd_host &ptd)
  {
    int const fd = _fileno(public_stream);

    if (fd < 0)
    {
      /* not an fd-backed stream (string streams and friends): the
       * original CRT owns it entirely */
      int const ret = ms::_fputc_nolock(c, public_stream);
      if (ret == EOF)
        ptd.get_errno().set(errno);
      return ret;
    }

    /* console check always comes first; the verdict is cached per
     * (fd, handle) snapshot inside the console object.  Inside a
     * guarded stdio call the object lock is already held and the fd
     * already resolved -- nothing per byte. */
    console *con = ptd.active_console();
    console::result r;

    if (con)
    {
      r = con->put(fd, static_cast<unsigned char>(c));
    }
    else
    {
      con = console::get(fd, true);
      if (!con)
      {
        /* allocation failure: get(fd, true) returns nullptr for this
         * and nothing else.  Fail closed (EOF) so UTF-8 output can
         * never leak onto the native ANSI path as mojibake. */
        ptd.get_errno().set(ENOMEM);
        ptd.get_doserrno().set(0);
        return EOF;
      }

      console::guard const g = con->acquire_guard();
      r = con->put(fd, static_cast<unsigned char>(c));
    }

    switch (r)
    {
    case console::result::ok:
      /* a parked partial sequence reports its bytes written, exactly
       * like the native _mbBuffer path */
      return static_cast<unsigned char>(c);

    case console::result::not_console:
      /* not a console (or the fd was recycled to a file mid-call):
       * this byte goes through the original CRT, which owns the FILE
       * buffer for files */
      {
        int const ret = ms::_fputc_nolock(c, public_stream);
        if (ret == EOF)
          ptd.get_errno().set(errno);
        return ret;
      }

    case console::result::error:
      ptd.get_errno().set(errno);
      return EOF;
    }

    return EOF;
  }

} // namespace mingw_thunk::ucrt
