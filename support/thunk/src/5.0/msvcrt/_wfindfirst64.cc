#include <thunk/_common.h>
#include <thunk/wntcrt/errno.h>
#include <thunk/wntcrt/time.h>

#include <io.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(intptr_t,
                     _wfindfirst64,
                     const wchar_t *filespec,
                     struct _wfinddata64_t *fileinfo)
  {
    if (const auto pfn = try_get__wfindfirst64())
      return pfn(filespec, fileinfo);

    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(filespec, &fd);
    if (h == INVALID_HANDLE_VALUE) {
      internal::dosmaperr(GetLastError());
      return -1;
    }

    ULARGE_INTEGER size = {
        .LowPart = fd.nFileSizeLow,
        .HighPart = fd.nFileSizeHigh,
    };

    fileinfo->attrib = fd.dwFileAttributes;
    fileinfo->time_create = internal::c_time64_from_filetime(fd.ftCreationTime);
    fileinfo->time_access =
        internal::c_time64_from_filetime(fd.ftLastAccessTime);
    fileinfo->time_write = internal::c_time64_from_filetime(fd.ftLastWriteTime);
    fileinfo->size = size.QuadPart;
    wcsncpy(fileinfo->name, fd.cFileName, MAX_PATH);

    return reinterpret_cast<intptr_t>(h);
  }
} // namespace mingw_thunk
