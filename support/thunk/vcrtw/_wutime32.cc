#include <thunk/_crt.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <sys/utime.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(int,
                     _wutime32,
                     const wchar_t *filename,
                     struct __utimbuf32 *times)
  {
    if (internal::is_nt())
      return crt__wutime32()(filename, times);

    stl::string a_path = internal::w2a(filename);
    return _utime32(a_path.c_str(), times);
  }
} // namespace mingw_thunk
