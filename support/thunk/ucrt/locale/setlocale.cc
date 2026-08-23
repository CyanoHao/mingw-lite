#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 setlocale,
                 int _Category,
                 char const *_Locale)
  {
    if (_Category < 0 || _Category > 5)
      return nullptr;

    const char *c_utf8 = "C.UTF-8";
    return const_cast<char *>(c_utf8);
  }
} // namespace mingw_thunk
