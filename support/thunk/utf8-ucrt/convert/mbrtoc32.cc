#include "../inc/corecrt_internal_mbstring.h"

#include <stdint.h>

namespace mingw_thunk::ucrt
{

  size_t __cdecl __crt_mbstring::__mbrtoc32_utf8(char32_t *pc32,
                                                 const char *s,
                                                 size_t n,
                                                 mbstate_t *ps,
                                                 __crt_cached_ptd_host &ptd)
  {
    const char *begin = s;
    static mbstate_t internal_pst{};
    if (ps == nullptr)
    {
      ps = &internal_pst;
    }

    if (!s)
    {
      s = "";
      n = 1;
      pc32 = nullptr;
    }

    if (n == 0)
    {
      return INCOMPLETE;
    }

    uint8_t length;
    uint8_t bytes_needed;
    char32_t c32;
    const bool init_state = (ps->_State == 0);
    if (init_state)
    {
      const uint8_t first_byte = static_cast<uint8_t>(*s++);

      if ((first_byte & 0x80) == 0)
      {
        if (pc32 != nullptr)
        {
          *pc32 = first_byte;
        }
        return first_byte != '\0' ? 1 : 0;
      }

      if ((first_byte & 0xe0) == 0xc0)
      {
        length = 2;
      }
      else if ((first_byte & 0xf0) == 0xe0)
      {
        length = 3;
      }
      else if ((first_byte & 0xf8) == 0xf0)
      {
        length = 4;
      }
      else
      {
        return return_illegal_sequence(ps, ptd);
      }
      bytes_needed = length;
      c32 = first_byte & ((1 << (7 - length)) - 1);
    }
    else
    {
      c32 = ps->_Wchar;
      length = static_cast<uint8_t>(ps->_Byte);
      bytes_needed = static_cast<uint8_t>(ps->_State);

      if (length < 2 || length > 4 || bytes_needed < 1 ||
          bytes_needed >= length)
      {
        return return_illegal_sequence(ps, ptd);
      }
    }

    if (bytes_needed < n)
    {
      n = bytes_needed;
    }

    while (static_cast<size_t>(s - begin) < n)
    {
      uint8_t current_byte = static_cast<uint8_t>(*s++);
      if ((current_byte & 0xc0) != 0x80)
      {
        return return_illegal_sequence(ps, ptd);
      }
      c32 = (c32 << 6) | (current_byte & 0x3f);
    }

    if (n < bytes_needed)
    {
      auto bytes_remaining = static_cast<uint8_t>(bytes_needed - n);
      static_assert(sizeof(mbstate_t::_Wchar) >= sizeof(char32_t),
                    "mbstate_t has broken mbrtoc32");
      ps->_Wchar = c32;
      ps->_Byte = length;
      ps->_State = bytes_remaining;
      return INCOMPLETE;
    }

    if ((0xd800 <= c32 && c32 <= 0xdfff) || (0x10ffff < c32))
    {
      return return_illegal_sequence(ps, ptd);
    }

    constexpr char32_t min_legal[3]{0x80, 0x800, 0x10000};
    if (c32 < min_legal[length - 2])
    {
      return return_illegal_sequence(ps, ptd);
    }

    if (pc32 != nullptr)
    {
      *pc32 = c32;
    }

    return reset_and_return(c32 == U'\0' ? 0 : bytes_needed, ps);
  }

} // namespace mingw_thunk::ucrt
