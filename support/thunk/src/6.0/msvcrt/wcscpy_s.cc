#include <thunk/_common.h>
#include <thunk/libc/string.h>
#include <thunk/libc/wchar.h>

#include <errno.h>
#include <stdint.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(
      errno_t, wcscpy_s, wchar_t *dest, rsize_t dest_size, const wchar_t *src)
  {
    if (const auto pfn = try_get_wcscpy_s())
      return pfn(dest, dest_size, src);

    constexpr size_t rsize_max = SIZE_MAX >> 1;
    constexpr size_t max_len = rsize_max / sizeof(wchar_t);

    if (!dest || !dest_size || dest_size >= max_len) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (!src) {
      *dest = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }

    size_t len = internal::wcsnlen(src, dest_size);

    if (len >= dest_size) {
      *dest = 0;
      _set_errno(ERANGE);
      return ERANGE;
    }

    internal::memcpy(dest, src, len * sizeof(wchar_t));
    dest[len] = 0;
    return 0;
  }
} // namespace mingw_thunk
