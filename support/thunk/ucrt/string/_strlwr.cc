#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <errno.h>

namespace mingw_thunk
{
  // In-place ASCII byte fold, >= 0x80 untouched (UTF-8 sequence
  // bytes are case-less); wine anchor: null -> NULL + EINVAL
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, char *, __cdecl, _strlwr, char *str)
  {
    if (!str) {
      _set_errno(EINVAL);
      return nullptr;
    }
    for (char *p = str; *p; ++p)
      *p = static_cast<char>(i::u8_byte_lower(static_cast<unsigned char>(*p)));
    return str;
  }
} // namespace mingw_thunk
