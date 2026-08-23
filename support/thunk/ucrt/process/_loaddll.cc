#include <thunk/_common.h>
#include <thunk/string.h>

namespace mingw_thunk
{
  // exec/loaddll.cpp, adapted: the reference is
  // LoadLibraryExA(szName, nullptr, 0), which is LoadLibraryA; our narrow
  // string is UTF-8, so the name is transcoded and the wide loader is
  // used.  A failed load returns 0 with the Win32 error left for
  // GetLastError (wine anchor: _loaddll of an unknown DLL is 0).
  // _unloaddll stays native -- it takes no string, so there is nothing to
  // re-anchor.
  __DEFINE_THUNK(api_ms_win_crt_process_l1_1_0,
                 0,
                 intptr_t,
                 __cdecl,
                 _loaddll,
                 char *name)
  {
    if (!name)
      return 0;

    d::w_str w_name;
    if (!w_name.from_u(name))
      return 0;

    return reinterpret_cast<intptr_t>(__ms_LoadLibraryW(w_name.c_str()));
  }
} // namespace mingw_thunk
