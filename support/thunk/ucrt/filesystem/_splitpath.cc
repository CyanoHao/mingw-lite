#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl _splitpath_s(const char *path,
                                          char *drive,
                                          size_t drive_count,
                                          char *directory,
                                          size_t directory_count,
                                          char *file_name,
                                          size_t file_name_count,
                                          char *extension,
                                          size_t extension_count);

  // reference/ucrt/filesystem/splitpath.cpp: slots may be null; the
  // fixed _MAX_* counts make the ERANGE paths effectively unreachable
  // and copies are unbounded.
  __DEFINE_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _splitpath,
                 const char *path,
                 char *drive,
                 char *directory,
                 char *file_name,
                 char *extension)
  {
    _splitpath_s(path,
                 drive, drive ? _MAX_DRIVE : 0,
                 directory, directory ? _MAX_DIR : 0,
                 file_name, file_name ? _MAX_FNAME : 0,
                 extension, extension ? _MAX_EXT : 0);
  }
} // namespace mingw_thunk
