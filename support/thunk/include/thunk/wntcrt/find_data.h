#include "../string.h"

#include <io.h>

#undef _findfirst
#undef _findfirst32
#undef _findfirst32i64
#undef _findfirst64
#undef _findfirst64i32
#undef _findfirsti64

#undef _findnext
#undef _findnext32
#undef _findnext32i64
#undef _findnext64
#undef _findnext64i32
#undef _findnexti64

extern "C"
{
  __attribute__((__dllimport__)) intptr_t
  _findfirst32(const char *filespec, struct _finddata32_t *fileinfo);
  __attribute__((__dllimport__)) intptr_t
  _findfirst32i64(const char *filespec, struct _finddata32i64_t *fileinfo);
  __attribute__((__dllimport__)) intptr_t
  _findfirst64(const char *filespec, struct __finddata64_t *fileinfo);
  __attribute__((__dllimport__)) intptr_t
  _findfirst64i32(const char *filespec, struct _finddata64i32_t *fileinfo);

  __attribute__((__dllimport__)) int
  _findnext32(intptr_t handle, struct _finddata32_t *fileinfo);
  __attribute__((__dllimport__)) int
  _findnext32i64(intptr_t handle, struct _finddata32i64_t *fileinfo);
  __attribute__((__dllimport__)) int
  _findnext64(intptr_t handle, struct __finddata64_t *fileinfo);
  __attribute__((__dllimport__)) int
  _findnext64i32(intptr_t handle, struct _finddata64i32_t *fileinfo);
}

namespace mingw_thunk
{
  namespace d
  {
    template <typename W>
    struct crt_find_data_a
    {
    };

    template <typename W>
    using crt_find_data_a_t = typename crt_find_data_a<W>::type;

    template <>
    struct crt_find_data_a<_wfinddata32_t>
    {
      using type = _finddata32_t;
    };

    template <>
    struct crt_find_data_a<_wfinddata32i64_t>
    {
      using type = _finddata32i64_t;
    };

    template <>
    struct crt_find_data_a<_wfinddata64_t>
    {
      using type = __finddata64_t;
    };

    template <>
    struct crt_find_data_a<_wfinddata64i32_t>
    {
      using type = _finddata64i32_t;
    };
  } // namespace d

  namespace i
  {
    template <typename W>
    d::crt_find_data_a_t<W> w2u(const W &w_file_info)
    {
      d::crt_find_data_a_t<W> u_file_info;

      u_file_info.attrib = w_file_info.attrib;
      u_file_info.time_create = w_file_info.time_create;
      u_file_info.time_access = w_file_info.time_access;
      u_file_info.time_write = w_file_info.time_write;
      u_file_info.size = w_file_info.size;
      d::u_str::best_effort_from_w(
          u_file_info.name, MAX_PATH, w_file_info.name);

      return u_file_info;
    }
  } // namespace i
} // namespace mingw_thunk
