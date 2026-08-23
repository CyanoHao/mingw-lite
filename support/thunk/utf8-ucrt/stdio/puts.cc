#include "../inc/corecrt_internal_stdio.h"

#include "../mingw/console.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace mingw_thunk::ucrt
{

  static int __cdecl _puts_internal(char const *const string,
                                    __crt_cached_ptd_host &ptd)
  {
    _UCRT_VALIDATE_RETURN(ptd, string != nullptr, EINVAL, EOF);

    FILE *const stream = stdout;
    _UCRT_VALIDATE_STREAM_ANSI_RETURN(ptd, stream, EINVAL, EOF);

    size_t const length = strlen(string);

    return __acrt_lock_stream_and_call(
        stream,
        [&]() -> int
        {
          __acrt_stdio_temporary_buffering_guard const buffering(stream, ptd);

          size_t const bytes_written =
              _fwrite_nolock_internal(string, 1, length, stream, ptd);

          if (bytes_written != length ||
              _fputc_nolock_internal('\n', stream, ptd) == EOF)
          {
            return EOF;
          }

          return 0;
        });
  }

  int __cdecl puts(char const *const string)
  {
    if (!console::is_console(1))
      return __ms_puts(string);

    __crt_cached_ptd_host ptd;
    return _puts_internal(string, ptd);
  }

} // namespace mingw_thunk::ucrt
