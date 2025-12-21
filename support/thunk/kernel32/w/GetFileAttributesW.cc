#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <errhandlingapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      kernel32, 4, DWORD, WINAPI, GetFileAttributesW, _In_ LPCWSTR lpFileName)
  {
    if (internal::is_nt())
      return get_GetFileAttributesW()(lpFileName);

    auto aname = internal::w2a(lpFileName);
    return kernel32_GetFileAttributesA()(aname.c_str());
  }
} // namespace mingw_thunk
