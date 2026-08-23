#pragma once

#include <stddef.h>
#include <wchar.h>

#include "musl.h"

namespace mingw_thunk
{
  namespace __crt_mbstring
  {
    constexpr size_t INVALID = static_cast<size_t>(-1);
    constexpr size_t INCOMPLETE = static_cast<size_t>(-2);

    inline size_t
    __mbrtoc32_utf8(char32_t *pc32, const char *s, size_t n, mbstate_t *ps)
    {
      return musl::mbrtoc32(
          pc32, s, n, reinterpret_cast<musl::mbstate_t *>(ps));
    }

    inline size_t __c16rtomb_utf8(char *s, char16_t c16, mbstate_t *ps)
    {
      return musl::c16rtomb(s, c16, reinterpret_cast<musl::mbstate_t *>(ps));
    }

    inline size_t __c32rtomb_utf8(char *s, char32_t c32, mbstate_t *ps)
    {
      return musl::c32rtomb(s, c32, reinterpret_cast<musl::mbstate_t *>(ps));
    }
  } // namespace __crt_mbstring
} // namespace mingw_thunk
