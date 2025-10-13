#pragma once

#include <stddef.h>

#include "../__config.h"

namespace NS_NOCRT
{
  inline wchar_t *wcspncpy(wchar_t *dst, const wchar_t *src, size_t dsize)
  {
    wchar_t *dend = dst + dsize;
    while (dst < dend && *src)
      *dst++ = *src++;
    wchar_t *ret = dst;
    while (dst < dend)
      *dst++ = 0;
    return ret;
  }

} // namespace NS_NOCRT
