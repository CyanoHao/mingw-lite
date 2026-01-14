#include <thunk/_crt.h>
#include <thunk/string.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                      int,
                      _findnext64,
                      intptr_t handle,
                      struct __finddata64_t *fileinfo)
  {
    _wfinddata64_t w_fileinfo;
    int res = _wfindnext64(res, &w_fileinfo);

    if (res == -1)
      return res;

    stl::string u_name = internal::w2u(w_fileinfo.name);

    fileinfo->attrib = w_fileinfo.attrib;
    fileinfo->time_create = w_fileinfo.time_create;
    fileinfo->time_access = w_fileinfo.time_access;
    fileinfo->time_write = w_fileinfo.time_write;
    fileinfo->size = w_fileinfo.size;

    if (u_name.size() >= MAX_PATH) {
      memcpy(fileinfo->name, u_name.c_str(), MAX_PATH - 1);
      fileinfo->name[MAX_PATH - 1] = 0;
    } else {
      memcpy(fileinfo->name, u_name.c_str(), u_name.size());
      fileinfo->name[u_name.size()] = 0;
    }
    return res;
  }
} // namespace mingw_thunk
