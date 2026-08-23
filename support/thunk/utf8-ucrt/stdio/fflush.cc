#include "../inc/corecrt_internal_stdio.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk::ucrt
{

  int __cdecl _fflush_nolock(FILE *const public_stream);

}

namespace mingw_thunk::ucrt
{

  int __cdecl fflush(FILE *const public_stream)
  {
    if (!public_stream)
    {
      console::flush_all();
      return __ms_fflush(nullptr);
    }

    if (!console::is_console(public_stream))
    {
      return __ms_fflush(public_stream);
    }

    return __acrt_lock_stream_and_call(
        public_stream, [&] { return ucrt::_fflush_nolock(public_stream); });
  }

  int __cdecl _fflush_nolock(FILE *const public_stream)
  {
    if (!public_stream)
    {
      console::flush_all();
      return __ms__fflush_nolock(nullptr);
    }

    if (!console::is_console(public_stream))
    {
      return __ms__fflush_nolock(public_stream);
    }

    console *c = console::get(public_stream, false);
    if (!c)
    {
      errno = EBADF;
      return EOF;
    }

    c->flush_nolock();
    return 0;
  }

} // namespace mingw_thunk::ucrt
