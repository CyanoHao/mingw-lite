#pragma once

#include <stdint.h>
#include <string.h>
#include <wchar.h>

namespace intl
{
  template <class CharT>
  class char_traits;

  template <>
  class char_traits<char>
  {
  public:
    using char_type = char;
    using int_type = int;

    static constexpr bool eq(char_type a, char_type b)
    {
      return static_cast<unsigned char>(a) == static_cast<unsigned char>(b);
    }

    static constexpr bool lt(char_type a, char_type b)
    {
      return static_cast<unsigned char>(a) < static_cast<unsigned char>(b);
    }

    static char_type *move(char_type *dest, const char_type *src, size_t count)
    {
      return static_cast<char_type *>(memmove(dest, src, count));
    }

    static char_type *copy(char_type *dest, const char_type *src, size_t count)
    {
      return static_cast<char_type *>(memcpy(dest, src, count));
    }

    static int compare(const char_type *s1, const char_type *s2, size_t count)
    {
      return memcmp(s1, s2, count);
    }

    static size_t length(const char_type *s)
    {
      return strlen(s);
    }
  };

  template <>
  class char_traits<wchar_t>
  {
  public:
    using char_type = wchar_t;
    using int_type = wint_t;

    static constexpr bool eq(char_type a, char_type b)
    {
      return a == b;
    }

    static constexpr bool lt(char_type a, char_type b)
    {
      return a < b;
    }

    static char_type *move(char_type *dest, const char_type *src, size_t count)
    {
      return wmemmove(dest, src, count);
    }

    static char_type *copy(char_type *dest, const char_type *src, size_t count)
    {
      return wmemcpy(dest, src, count);
    }

    static int compare(const char_type *s1, const char_type *s2, size_t count)
    {
      return wmemcmp(s1, s2, count);
    }

    static size_t length(const char_type *s)
    {
      return wcslen(s);
    }
  };
} // namespace intl
