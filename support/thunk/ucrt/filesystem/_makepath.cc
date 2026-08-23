#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl _makepath_s(char *result_buffer,
                                         size_t result_count,
                                         const char *drive,
                                         const char *directory,
                                         const char *file_name,
                                         const char *extension);

  // Unbounded variant per the reference shell: count = (size_t)-1
  // disables every bounds check.
  __DEFINE_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _makepath,
                 char *result_buffer,
                 const char *drive,
                 const char *directory,
                 const char *file_name,
                 const char *extension)
  {
    _makepath_s(result_buffer, static_cast<size_t>(-1), drive, directory,
                file_name, extension);
  }
} // namespace mingw_thunk
