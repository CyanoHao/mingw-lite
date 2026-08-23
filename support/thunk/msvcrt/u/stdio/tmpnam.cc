#include <thunk/_common.h>
#include <thunk/string.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, char *, __cdecl, tmpnam, char *buffer)
  {
    wchar_t w_buffer[L_tmpnam];
    const wchar_t *w_name = _wtmpnam(buffer ? w_buffer : nullptr);
    if (!w_name)
      return nullptr;

    static char internal_buffer[L_tmpnam];
    char *destination = buffer ? buffer : internal_buffer;

    if (!d::u_str::best_effort_from_w(destination, L_tmpnam, w_name))
      return nullptr;

    return destination;
  }
} // namespace mingw_thunk
