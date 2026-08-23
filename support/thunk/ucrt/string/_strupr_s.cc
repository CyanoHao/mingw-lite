#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <errno.h>

namespace mingw_thunk
{
  // wine anchors: size 0 / null -> EINVAL with the buffer
  // UNTOUCHED; unterminated-in-size / too small -> EINVAL with
  // str[0] reset; success -> 0 in place
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, errno_t, __cdecl, _strupr_s, char *str, size_t size)
  {
    if (!str || size == 0) {
      _set_errno(EINVAL);
      return EINVAL;
    }
    size_t length = 0;
    while (length < size && str[length])
      ++length;
    if (length == size) {
      str[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }
    for (size_t k = 0; k < length; k++)
      str[k] = static_cast<char>(i::u8_byte_upper(static_cast<unsigned char>(str[k])));
    return 0;
  }
} // namespace mingw_thunk
