#include "../inc/corecrt_internal_mbstring.h"
#include "../inc/corecrt_internal_ptd_propagation.h"

#include <stdint.h>

namespace mingw_thunk::ucrt
{

  using namespace __crt_mbstring;

  size_t __cdecl c32rtomb(char *s, char32_t c32, mbstate_t *ps)
  {
    __crt_cached_ptd_host ptd;
    return __c32rtomb_utf8(s, c32, ps, ptd);
  }

  size_t __cdecl __crt_mbstring::__c32rtomb_utf8(char *s,
                                                 char32_t c32,
                                                 mbstate_t *ps,
                                                 __crt_cached_ptd_host &ptd)
  {
    if (!s)
    {
      *ps = {};
      return 1;
    }

    if (c32 == U'\0')
    {
      *s = '\0';
      *ps = {};
      return 1;
    }

    if ((c32 & ~0x7f) == 0)
    {
      *s = static_cast<char>(c32);
      return 1;
    }

    size_t trail_bytes;
    uint8_t lead_byte;
    if ((c32 & ~0x7ff) == 0)
    {
      trail_bytes = 1;
      lead_byte = 0xc0;
    }
    else if ((c32 & ~0xffff) == 0)
    {
      if (0xd800 <= c32 && c32 <= 0xdfff)
      {
        return return_illegal_sequence(ps, ptd);
      }
      trail_bytes = 2;
      lead_byte = 0xe0;
    }
    else if ((c32 & ~0x001fffff) == 0)
    {
      if (0x10ffff < c32)
      {
        return return_illegal_sequence(ps, ptd);
      }
      trail_bytes = 3;
      lead_byte = 0xf0;
    }
    else
    {
      return return_illegal_sequence(ps, ptd);
    }

    for (size_t i = trail_bytes; i > 0; --i)
    {
      s[i] = (c32 & 0x3f) | 0x80;
      c32 >>= 6;
    }

    s[0] = static_cast<uint8_t>(c32) | lead_byte;

    return reset_and_return(trail_bytes + 1, ps);
  }

} // namespace mingw_thunk::ucrt
