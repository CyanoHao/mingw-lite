#include "_wtmpnam.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, wchar_t *, __cdecl, _wtmpnam, wchar_t *buffer)
  {
    __DISPATCH_THUNK_2(
        _wtmpnam, i::is_nt(), &__ms__wtmpnam, &f::win9x__wtmpnam);

    return dllimport__wtmpnam(buffer);
  }

  namespace f
  {
    wchar_t *win9x__wtmpnam(wchar_t *buffer)
    {
      char a_buffer[L_tmpnam];
      const char *a_name = __ms_tmpnam(buffer ? a_buffer : nullptr);
      if (!a_name)
        return nullptr;

      static wchar_t internal_buffer[L_tmpnam];
      wchar_t *destination = buffer ? buffer : internal_buffer;

      if (!d::w_str::best_effort_from_a(destination, L_tmpnam, a_name))
        return nullptr;

      return destination;
    }
  } // namespace f
} // namespace mingw_thunk
