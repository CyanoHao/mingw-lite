#include "../inc/corecrt_internal_ptd_propagation.h"
#include "../inc/corecrt_internal_stdio.h"

namespace mingw_thunk::ucrt
{

  wint_t __cdecl _fputwc_nolock_internal(wchar_t const c,
                                         FILE *const public_stream,
                                         __crt_cached_ptd_host &ptd)
  {
    __crt_stdio_stream const stream(public_stream);

    // UCRT checks for string-backed stream leftover from legacy VCRT.

    __crt_lowio_text_mode const text_mode =
        _textmode_safe(_fileno(stream.public_stream()));

    if (text_mode == __crt_lowio_text_mode::utf16le ||
        text_mode == __crt_lowio_text_mode::utf8)
    {
      return fputwc_binary_nolock(c, stream, ptd);
    }

    if ((_osfile_safe(_fileno(stream.public_stream())) & FTEXT) == 0)
    {
      return fputwc_binary_nolock(c, stream, ptd);
    }

    char mbc[MB_LEN_MAX];

    int size;
    if (_wctomb_internal(&size, mbc, MB_LEN_MAX, c, ptd) != 0)
    {
      return WEOF;
    }

    for (int i = 0; i < size; ++i)
    {
      if (_fputc_nolock_internal(mbc[i], stream.public_stream(), ptd) == EOF)
      {
        return WEOF;
      }
    }

    return c;
  }

} // namespace mingw_thunk::ucrt
