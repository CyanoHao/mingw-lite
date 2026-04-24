#pragma once

#include "char_traits.h"

namespace intl
{
  template <class CharT, class Traits = char_traits<CharT>>
  class basic_string_view
  {
  public:
    using value_type = CharT;
    using const_pointer = const CharT *;
    using const_iterator = const CharT *;
    using size_type = size_t;

  public:
    static constexpr size_type npos = size_type(-1);

  private:
    const_pointer data_;
    size_type size_;

  public:
    constexpr basic_string_view() noexcept : data_(nullptr), size_(0)
    {
    }

    constexpr basic_string_view(const basic_string_view &other) noexcept =
        default;

    constexpr basic_string_view(const CharT *s, size_type count)
        : data_(s), size_(count)
    {
    }

    constexpr basic_string_view(const CharT *s)
        : data_(s), size_(Traits::length(s))
    {
    }

    constexpr basic_string_view &
    operator=(const basic_string_view &view) noexcept = default;

    constexpr const CharT &operator[](size_type pos) const
    {
      return data_[pos];
    }

    constexpr const_pointer data() const noexcept
    {
      return data_;
    }

    constexpr size_type size() const noexcept
    {
      return size_;
    }

    constexpr size_type length() const noexcept
    {
      return size_;
    }

    constexpr bool empty() const noexcept
    {
      return size_ == 0;
    }

    constexpr basic_string_view substr(size_type pos = 0,
                                       size_type count = npos) const
    {
      size_type rlen = count < (size_ - pos) ? count : (size_ - pos);
      return basic_string_view(data_ + pos, rlen);
    }

    constexpr int compare(basic_string_view v) const noexcept
    {
      size_type rlen = size_ < v.size_ ? size_ : v.size_;
      int res = Traits::compare(data_, v.data_, rlen);
      if (res != 0)
        return res;
      else if (size_ < v.size_)
        return -1;
      else if (size_ == v.size_)
        return 0;
      else
        return 1;
    }

    constexpr size_type find(basic_string_view v,
                             size_type pos = 0) const noexcept
    {
      for (size_type i = pos; i + v.size() <= size(); ++i) {
        if (Traits::compare(data() + i, v.data(), v.size()) == 0)
          return i;
      }
      return npos;
    }

    constexpr size_type find(CharT ch, size_type pos = 0) const noexcept
    {
      for (size_type i = pos; i < size(); ++i) {
        if (data()[i] == ch)
          return i;
      }
      return npos;
    }

    constexpr size_type find_first_of(basic_string_view v,
                                      size_type pos = 0) const noexcept
    {
      for (size_type i = pos; i < size(); ++i) {
        if (v.find(data()[i]) != v.npos)
          return i;
      }
      return npos;
    }

    constexpr size_type rfind(CharT ch, size_type pos = npos) const noexcept
    {
      if (empty())
        return npos;
      if (pos == npos || pos >= size())
        pos = size() - 1;
      for (size_type i = 0; i <= pos; ++i) {
        size_type idx = pos - i;
        if (Traits::eq(data()[idx], ch))
          return idx;
      }
      return npos;
    }
  };

  template <class CharT, class Traits>
  constexpr bool operator==(basic_string_view<CharT, Traits> lhs,
                            basic_string_view<CharT, Traits> rhs) noexcept
  {
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
  }

  template <class CharT, class Traits>
  constexpr bool operator!=(basic_string_view<CharT, Traits> lhs,
                            basic_string_view<CharT, Traits> rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <class CharT, class Traits>
  constexpr bool operator<(basic_string_view<CharT, Traits> lhs,
                           basic_string_view<CharT, Traits> rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }

  template <class CharT, class Traits>
  constexpr bool operator==(basic_string_view<CharT, Traits> lhs,
                            const CharT *rhs) noexcept
  {
    return lhs == basic_string_view<CharT, Traits>(rhs);
  }

  template <class CharT, class Traits>
  constexpr bool operator!=(basic_string_view<CharT, Traits> lhs,
                            const CharT *rhs) noexcept
  {
    return !(lhs == rhs);
  }

  using string_view = basic_string_view<char>;
  using wstring_view = basic_string_view<wchar_t>;
} // namespace intl
