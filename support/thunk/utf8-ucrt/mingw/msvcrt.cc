#include "msvcrt.h"

#include <thunk/_no_thunk.h>

#include <stdio.h>

#ifdef _UCRT

namespace mingw_thunk::ucrt::ms
{

  int _fputc_nolock(int c, FILE *stream)
  {
    return __ms__fputc_nolock(c, stream);
  }

  int _fputwc_nolock(wchar_t wc, FILE *stream)
  {
    return __ms__fputwc_nolock(wc, stream);
  }

} // namespace mingw_thunk::ucrt::ms

#else

namespace mingw_thunk::ucrt::ms
{

  int _fputc_nolock(int c, FILE *stream)
  {
    return --stream->_cnt >= 0 ? 0xff & (*stream->_ptr++ = (char)c)
                               : _flsbuf(c, stream);
  }

  int _fputwc_nolock(wchar_t wc, FILE *stream)
  {
    unsigned char b0 = wc;
    unsigned char b1 = wc >> 8;

    stream->_cnt -= 2;

    if (stream->_cnt >= 0)
    {
      *stream->_ptr++ = b0;
      *stream->_ptr++ = b1;
      return wc;
    }

    if (_flsbuf(b0, stream) == EOF)
      return WEOF;
    if (_flsbuf(b1, stream) == EOF)
      return WEOF;
    return wc;
  }

} // namespace mingw_thunk::ucrt::ms

#endif
