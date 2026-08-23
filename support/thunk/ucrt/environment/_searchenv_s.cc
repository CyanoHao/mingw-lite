#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/musl.h>

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // reference/ucrt/env/searchenv.cpp walk, wine-anchored (probe D + r14):
  //  - validation order buf -> count -> file_name, buffers UNTOUCHED on
  //    EINVAL (wine; the reference shell clears first — wine is the
  //    oracle)
  //  - empty file name: reference semantics (buf cleared + ENOENT);
  //    wine leaves buf[0]='C'-garbage with errno 0 — wine shell-bug
  //    candidate, R-1 noted
  //  - cwd-relative hit -> _fullpath (probe: subdir component hit stays
  //    relative — only the direct hit is fully qualified)
  //  - missing env var / exhausted components -> ENOENT
  //  - capacity miss -> ERANGE
  // Components are split on ';', empty ones skipped, '"' dropped
  // (reference getpath); each probed as dir + '\\' + name.
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _searchenv_s,
                 const char *file_name,
                 const char *environment_variable,
                 char *result_buffer,
                 size_t result_count)
  {
    if (!result_buffer || result_count == 0 || !file_name) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (file_name[0] == '\0') {
      result_buffer[0] = '\0';
      _set_errno(ENOENT);
      return ENOENT;
    }

    int saved_errno = errno;

    // Direct hit in the current directory -> fully qualified path.
    {
      d::w_str wide_file;
      if (wide_file.from_u(file_name) &&
          _waccess(wide_file.c_str(), 0) == 0) {
        errno = saved_errno;
        if (!_fullpath(result_buffer, file_name, result_count)) {
          result_buffer[0] = '\0';
          return errno; // _fullpath set ERANGE/ENOMEM
        }
        return 0;
      }
    }

    const char *paths = musl::getenv(environment_variable);
    if (!paths) {
      result_buffer[0] = '\0';
      _set_errno(ENOENT);
      return ENOENT;
    }

    size_t file_length = strlen(file_name);
    size_t component_capacity = strlen(paths) + file_length + 2;

    char *component =
        static_cast<char *>(malloc(component_capacity));
    if (!component) {
      result_buffer[0] = '\0';
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    errno_t result = ENOENT;

    const char *walk = paths;
    for (;;) {
      while (*walk == ';')
        ++walk;
      if (*walk == '\0')
        break;

      // Copy the component, dropping quote characters (getpath).
      size_t length = 0;
      while (*walk != '\0' && *walk != ';') {
        if (*walk != '"')
          component[length++] = *walk;
        ++walk;
      }
      component[length] = '\0';

      // Append a separator and the file name, then probe.
      if (length == 0 || (component[length - 1] != '/' &&
                          component[length - 1] != '\\' &&
                          component[length - 1] != ':'))
        component[length++] = '\\';
      memcpy(component + length, file_name, file_length + 1);

      d::w_str wide_component;
      if (wide_component.from_u(component) &&
          _waccess(wide_component.c_str(), 0) == 0) {
        size_t full_length = length + file_length + 1;
        if (full_length > result_count) {
          result_buffer[0] = '\0';
          _set_errno(ERANGE);
          free(component);
          return ERANGE;
        }

        errno = saved_errno;
        memcpy(result_buffer, component, full_length);
        free(component);
        return 0;
      }
    }

    result_buffer[0] = '\0';
    _set_errno(result);
    free(component);
    return result;
  }
} // namespace mingw_thunk
