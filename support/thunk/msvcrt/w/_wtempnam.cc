#include "_wtempnam.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 wchar_t *,
                 __cdecl,
                 _wtempnam,
                 const wchar_t *dir,
                 const wchar_t *prefix)
  {
    __DISPATCH_THUNK_2(
        _wtempnam, i::is_nt(), &__ms__wtempnam, &f::win9x__wtempnam);

    return dllimport__wtempnam(dir, prefix);
  }

  namespace f
  {
    wchar_t *win9x__wtempnam(const wchar_t *dir, const wchar_t *prefix)
    {
      d::a_str a_dir;
      if (dir && !a_dir.from_w(dir)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      d::a_str a_prefix;
      if (prefix && !a_prefix.from_w(prefix)) {
        _set_errno(ENOMEM);
        return nullptr;
      }

      char *a_result = __ms__tempnam(dir ? a_dir.c_str() : nullptr,
                                     prefix ? a_prefix.c_str() : nullptr);
      if (!a_result)
        return nullptr;

      d::w_str w_result;
      if (!w_result.from_a(a_result)) {
        free(a_result);
        _set_errno(ENOMEM);
        return nullptr;
      }

      // the caller frees the result with free(), use the CRT allocator
      wchar_t *result = static_cast<wchar_t *>(
          malloc((w_result.size() + 1) * sizeof(wchar_t)));
      if (result)
        c::wcscpy(result, w_result.c_str());
      else
        _set_errno(ENOMEM);

      free(a_result);
      return result;
    }
  } // namespace f
} // namespace mingw_thunk
