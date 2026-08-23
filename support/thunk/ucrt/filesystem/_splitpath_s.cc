#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // reference/ucrt/filesystem/splitpath.cpp, UTF-8 edition.  The
  // reference calls _ismbblead() while scanning so DBCS trail bytes
  // that "look like" separators are skipped; native byte semantics
  // drift with the mbcp state (probe E: 0xC2 before '\' is eaten under
  // _setmbcp(936)).  In UTF-8, continuation bytes are always >= 0x80
  // and can never equal '/', '\\' or '.', so the plain byte scan IS
  // the correct self-synchronizing walk (R1 reversal).
  //
  // wine anchors (probe E + r12/r13): EINVAL (null path, or a
  // (pointer,count) pair mismatch) leaves every buffer UNTOUCHED (the
  // reference shell would clear them — wine is the oracle); ERANGE
  // (a component does not fit) clears ALL non-null buffers; the drive
  // needs count >= 3 ("C:" + NUL); a dot inside the directory part
  // does not split the name; ".bashrc" -> file "" ext ".bashrc";
  // "file." -> file "file" ext ".".
  namespace
  {
    errno_t splitpath_core(const char *path,
                           char *drive,
                           size_t drive_count,
                           char *directory,
                           size_t directory_count,
                           char *file_name,
                           size_t file_name_count,
                           char *extension,
                           size_t extension_count,
                           bool bounded)
    {
      auto mismatch = [](char *buffer, size_t count) {
        return (buffer == nullptr) != (count == 0);
      };

      if (!path || mismatch(drive, drive_count) ||
          mismatch(directory, directory_count) ||
          mismatch(file_name, file_name_count) ||
          mismatch(extension, extension_count)) {
        _set_errno(EINVAL);
        return EINVAL;
      }

      auto clear_all = [&]() {
        if (drive)
          drive[0] = '\0';
        if (directory)
          directory[0] = '\0';
        if (file_name)
          file_name[0] = '\0';
        if (extension)
          extension[0] = '\0';
        _set_errno(ERANGE);
        return ERANGE;
      };

      auto copy_component = [&](char *dest, size_t count, const char *begin,
                                size_t length) -> bool {
        if (!dest)
          return true;
        if (bounded && count <= length) {
          clear_all();
          return false;
        }
        for (size_t i = 0; i < length; ++i)
          dest[i] = begin[i];
        dest[length] = '\0';
        return true;
      };

      auto reset_component = [&](char *dest) {
        if (dest)
          dest[0] = '\0';
      };

      const char *path_it = path;

      // Drive letter: "X:" checked at position 1 (_MAX_DRIVE - 2 = 1).
      if (path[0] != '\0' && path[1] == ':') {
        if (drive) {
          if (bounded && drive_count < _MAX_DRIVE)
            return clear_all();
          copy_component(drive, drive_count, path, 2);
        }
        path_it = path + 2;
      } else {
        reset_component(drive);
      }

      // Scan for the last separator and the last dot (plain bytes —
      // see the header comment).
      const char *p = path_it;
      const char *last_slash = nullptr;
      const char *last_dot = nullptr;
      while (*p != '\0') {
        if (*p == '/' || *p == '\\')
          last_slash = p + 1;
        else if (*p == '.')
          last_dot = p;
        ++p;
      }

      if (last_slash) {
        if (!copy_component(directory, directory_count, path_it,
                            size_t(last_slash - path_it)))
          return ERANGE;
        path_it = last_slash;
      } else {
        reset_component(directory);
      }

      if (last_dot && last_dot >= path_it) {
        if (!copy_component(file_name, file_name_count, path_it,
                            size_t(last_dot - path_it)))
          return ERANGE;
        if (!copy_component(extension, extension_count, last_dot,
                            size_t(p - last_dot)))
          return ERANGE;
      } else {
        if (!copy_component(file_name, file_name_count, path_it,
                            size_t(p - path_it)))
          return ERANGE;
        reset_component(extension);
      }

      return 0;
    }
  } // namespace

  __DEFINE_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _splitpath_s,
                 const char *path,
                 char *drive,
                 size_t drive_count,
                 char *directory,
                 size_t directory_count,
                 char *file_name,
                 size_t file_name_count,
                 char *extension,
                 size_t extension_count)
  {
    return splitpath_core(path, drive, drive_count, directory,
                          directory_count, file_name, file_name_count,
                          extension, extension_count, true);
  }
} // namespace mingw_thunk
