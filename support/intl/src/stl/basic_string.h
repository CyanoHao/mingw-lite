#pragma once

#include "basic_string_view.h"
#include "char_traits.h"

#include <stdlib.h>

namespace intl
{
  template <class CharT, class Traits = char_traits<CharT>>
  class basic_string
  {
  public:
    using value_type = CharT;
    using size_type = size_t;
    using const_pointer = const CharT *;
    using pointer = CharT *;
    using const_iterator = const CharT *;
    using iterator = CharT;

  public:
    static constexpr size_type npos = size_type(-1);

  private:
    pointer data_;
    size_type size_;
    size_type capacity_;

  public:
    void *operator new(size_t size, basic_string *p)
    {
      return p;
    }

  public:
    basic_string() : data_(nullptr), size_(0), capacity_(0)
    {
      if (!realloc(1))
        return;
      data_[0] = CharT(0);
    }

    basic_string(size_type count, CharT ch)
        : data_(nullptr), size_(0), capacity_(0)
    {
      if (!realloc(count + 1))
        return;
      size_ = count;
      for (size_type i = 0; i < count; ++i)
        data_[i] = ch;
      data_[count] = CharT(0);
    }

    basic_string(const CharT *s, size_type count)
        : data_(nullptr), size_(0), capacity_(0)
    {
      if (!realloc(count + 1))
        return;
      size_ = count;
      Traits::copy(data_, s, count);
      data_[count] = CharT(0);
    }

    basic_string(const CharT *s) : basic_string(s, Traits::length(s))
    {
    }

    basic_string(const basic_string &other)
        : data_(nullptr), size_(0), capacity_(0)
    {
      if (!realloc(other.size_ + 1))
        return;
      size_ = other.size_;
      Traits::copy(data_, other.data_, size_);
      data_[size_] = CharT(0);
    }

    basic_string(basic_string &&other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }

    ~basic_string()
    {
      free(data_);
    }

    basic_string &operator=(const basic_string &str)
    {
      if (this == &str)
        return *this;
      basic_string tmp(str);
      swap(tmp);
      return *this;
    }

    basic_string &operator=(basic_string &&str)
    {
      if (this == &str)
        return *this;
      free(data_);
      data_ = str.data_;
      size_ = str.size_;
      capacity_ = str.capacity_;
      str.data_ = nullptr;
      str.size_ = 0;
      str.capacity_ = 0;
      return *this;
    }

    // Element access
    CharT &operator[](size_type pos)
    {
      return data_[pos];
    }

    const CharT &operator[](size_type pos) const
    {
      return data_[pos];
    }

    const CharT &front() const
    {
      return data_[0];
    }

    CharT &front()
    {
      return data_[0];
    }

    const CharT &back() const
    {
      return data_[size_ - 1];
    }

    CharT &back()
    {
      return data_[size_ - 1];
    }

    const CharT *data() const
    {
      return data_;
    }

    CharT *data()
    {
      return data_;
    }

    const CharT *c_str() const
    {
      return data_;
    }

    operator basic_string_view<CharT, Traits>() const
    {
      if (!valid())
        return basic_string_view<CharT, Traits>();
      return basic_string_view<CharT, Traits>(data_, size_);
    }

    // Status
    bool valid() const
    {
      return data_ != nullptr;
    }

    // Capacity
    bool empty() const
    {
      return size_ == 0;
    }

    size_type size() const
    {
      return size_;
    }

    size_type length() const
    {
      return size_;
    }

    // Modifiers
    void clear()
    {
      if (data_) {
        data_[0] = CharT(0);
        size_ = 0;
      }
    }

    void push_back(CharT ch)
    {
      if (!valid())
        return;
      append(1, ch);
    }

    basic_string &append(const CharT *s, size_type count)
    {
      if (!valid())
        return *this;
      size_type old_size = size_;
      resize(old_size + count);
      if (data_)
        Traits::copy(data_ + old_size, s, count);
      return *this;
    }

    basic_string &append(size_type count, CharT ch)
    {
      if (!valid())
        return *this;
      resize(size_ + count, ch);
      return *this;
    }

    basic_string &append(const basic_string_view<CharT, Traits> &sv)
    {
      return append(sv.data(), sv.size());
    }

    basic_string &operator+=(const basic_string_view<CharT, Traits> &sv)
    {
      return append(sv.data(), sv.size());
    }

    basic_string &operator+=(const CharT *s)
    {
      return append(s, Traits::length(s));
    }

    basic_string &operator+=(CharT ch)
    {
      push_back(ch);
      return *this;
    }

    void resize(size_type count)
    {
      resize(count, CharT());
    }

    void resize(size_type count, CharT ch)
    {
      if (!valid())
        return;
      if (count <= capacity_) {
        // No reallocation needed
        if (count > size_) {
          for (size_type i = size_; i < count; ++i)
            data_[i] = ch;
        }
        size_ = count;
        if (data_)
          data_[size_] = CharT(0);
      } else {
        // Need reallocation
        if (!realloc(count + 1))
          return;
        if (count > size_) {
          for (size_type i = size_; i < count; ++i)
            data_[i] = ch;
        }
        size_ = count;
        data_[size_] = CharT(0);
      }
    }

    void swap(basic_string &other)
    {
      pointer tmp_data = other.data_;
      size_type tmp_size = other.size_;
      size_type tmp_capacity = other.capacity_;

      other.data_ = data_;
      other.size_ = size_;
      other.capacity_ = capacity_;

      data_ = tmp_data;
      size_ = tmp_size;
      capacity_ = tmp_capacity;
    }

    // String operations
    int compare(const basic_string &str) const
    {
      if (!valid() && !str.valid())
        return 0;
      if (!valid())
        return -1;
      if (!str.valid())
        return 1;
      size_type rlen = size_ < str.size_ ? size_ : str.size_;
      int res = Traits::compare(data_, str.data_, rlen);
      if (res != 0)
        return res;
      else if (size_ < str.size_)
        return -1;
      else if (size_ > str.size_)
        return 1;
      else
        return 0;
    }

    size_type rfind(CharT ch, size_type pos = npos) const
    {
      if (!valid())
        return npos;
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

    size_type rfind(const basic_string_view<CharT, Traits> &sv,
                    size_type pos = npos) const
    {
      if (!valid())
        return npos;
      if (sv.empty())
        return pos < size() ? pos : size();
      if (sv.size() > size())
        return npos;

      if (pos == npos || pos > size() - sv.size())
        pos = size() - sv.size();
      for (size_type i = 0; i <= pos; ++i) {
        size_type idx = pos - i;
        if (Traits::compare(data() + idx, sv.data(), sv.size()) == 0)
          return idx;
      }
      return npos;
    }

    size_type find(CharT ch, size_type pos = 0) const noexcept
    {
      if (!valid())
        return npos;
      for (size_type i = pos; i < size(); ++i) {
        if (data()[i] == ch)
          return i;
      }
      return npos;
    }

    size_type find_first_of(basic_string_view<CharT, Traits> v,
                            size_type pos = 0) const noexcept
    {
      if (!valid())
        return npos;
      for (size_type i = pos; i < size(); ++i) {
        if (v.find(data()[i]) != v.npos)
          return i;
      }
      return npos;
    }

  private:
    bool realloc(size_type new_capacity)
    {
      pointer new_data =
          static_cast<pointer>(::realloc(data_, new_capacity * sizeof(CharT)));
      if (!new_data) {
        // On failure, enter empty state
        free(data_);
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        return false;
      }
      data_ = new_data;
      capacity_ = new_capacity;
      return true;
    }
  };

  // String comparison operators
  template <class CharT, class Traits>
  bool operator==(const basic_string<CharT, Traits> &lhs,
                  const basic_string<CharT, Traits> &rhs) noexcept
  {
    if (!lhs.valid() && !rhs.valid())
      return true;
    if (!lhs.valid() || !rhs.valid())
      return false;
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
  }

  template <class CharT, class Traits>
  bool operator!=(const basic_string<CharT, Traits> &lhs,
                  const basic_string<CharT, Traits> &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <class CharT, class Traits>
  bool operator<(const basic_string<CharT, Traits> &lhs,
                 const basic_string<CharT, Traits> &rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }

  using string = basic_string<char>;
  using wstring = basic_string<wchar_t>;
} // namespace intl
