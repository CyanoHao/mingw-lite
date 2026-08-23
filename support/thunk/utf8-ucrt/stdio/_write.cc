#include "../mingw/console.h"

#include <thunk/_no_thunk.h>

#include <errno.h>
#include <io.h>

namespace mingw_thunk::ucrt
{

  int __cdecl
  _write(int const fd, void const *const buffer, unsigned int const count)
  {
    if (!console::is_console(fd))
      return __ms__write(fd, buffer, count);

    console *const con = console::get(fd, true);
    if (!con)
    {
      errno = EBADF;
      return -1;
    }

    auto guard = con->acquire_guard();
    return static_cast<int>(con->write_nolock(buffer, count));
  }

} // namespace mingw_thunk::ucrt
