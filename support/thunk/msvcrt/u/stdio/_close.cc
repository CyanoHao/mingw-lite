#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, _close, int fd)
  {
    /* release (and flush) the console channel while the fd is still
     * open; fd 0/1/2 only flush the static trio */
    musl::console_channel_release(fd);
    return __ms__close(fd);
  }
} // namespace mingw_thunk
