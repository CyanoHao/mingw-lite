#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      kernel32, 4, BOOL, WINAPI, RemoveDirectoryW, _In_ LPCWSTR lpPathName)
  {
    if (internal::is_nt())
      return get_RemoveDirectoryW()(lpPathName);

    auto aname = internal::w2a(lpPathName);
    return kernel32_RemoveDirectoryA()(aname.c_str());
  }
} // namespace mingw_thunk
