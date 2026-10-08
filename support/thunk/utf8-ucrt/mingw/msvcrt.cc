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

} // namespace mingw_thunk::ucrt::ms

#else

namespace mingw_thunk::ucrt::ms
{

  int _fputc_nolock(int c, FILE *stream)
  {
    return --stream->_cnt >= 0 ? 0xff & (*stream->_ptr++ = (char)c)
                               : _flsbuf(c, stream);
  }

} // namespace mingw_thunk::ucrt::ms

#endif
