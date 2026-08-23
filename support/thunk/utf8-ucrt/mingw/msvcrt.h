#pragma once

#include <stdio.h>

namespace mingw_thunk::ucrt::ms
{

  int _fputc_nolock(int c, FILE *stream);
  int _fputwc_nolock(wchar_t wc, FILE *stream);

}
