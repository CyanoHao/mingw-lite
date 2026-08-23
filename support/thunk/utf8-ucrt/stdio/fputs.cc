#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace mingw_thunk::ucrt
{

  static int __cdecl _fputs_internal(char const *const string,
                                     FILE *const stream,
                                     __crt_cached_ptd_host &ptd)
  {
    _UCRT_VALIDATE_RETURN(ptd, string != nullptr, EINVAL, EOF);
    _UCRT_VALIDATE_RETURN(ptd, stream != nullptr, EINVAL, EOF);
    _UCRT_VALIDATE_STREAM_ANSI_RETURN(ptd, stream, EINVAL, EOF);

    size_t const length = strlen(string);

    return __acrt_lock_stream_and_call(
        stream,
        [&]() -> int
        {
          __acrt_stdio_temporary_buffering_guard const buffering(stream, ptd);

          size_t const bytes_written =
              _fwrite_nolock_internal(string, 1, length, stream, ptd);
          if (bytes_written == length)
          {
            return 0;
          }

          return EOF;
        });
  }

  int __cdecl fputs(char const *const string, FILE *const stream)
  {
    if (!console::is_console(stream))
      return __ms_fputs(string, stream);

    __crt_cached_ptd_host ptd;
    return _fputs_internal(string, stream, ptd);
  }

} // namespace mingw_thunk::ucrt
