#pragma once

#include <stddef.h>

namespace std
{
  template <typename E>
  class initializer_list {
  public:
    using value_type = E;
    using reference = const E &;
    using const_reference = const E &;
    using size_type = size_t;
    using iterator = const E *;
    using const_iterator = const E *;

  private:
    iterator mpArray;
    size_type mArraySize;

    initializer_list(const_iterator pArray, size_type arraySize)
        : mpArray(pArray), mArraySize(arraySize)
    {
    }

  public:
    initializer_list() noexcept : mpArray(nullptr), mArraySize(0) {}

    size_type size() const noexcept { return mArraySize; }
    const_iterator begin() const noexcept { return mpArray; }
    const_iterator end() const noexcept { return mpArray + mArraySize; }
  };

  template <class T>
  const T *begin(std::initializer_list<T> ilist) noexcept
  {
    return ilist.begin();
  }

  template <class T>
  const T *end(std::initializer_list<T> ilist) noexcept
  {
    return ilist.end();
  }
} // namespace std
