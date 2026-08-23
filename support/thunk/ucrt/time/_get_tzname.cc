#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <string.h>

namespace mingw_thunk
{
  // wine anchors (2026-09-27, M9 probe):
  //  - index outside {0,1} or null buffer -> EINVAL; the out-params
  //    stay UNTOUCHED (retval kept its poison, buffer kept content)
  //  - buffer too small -> ERANGE (34) with buf[0] reset when the
  //    buffer has room for it; errno NOT raised
  //  - success -> rc 0, *retval = required bytes INCLUDING the NUL
  //    ("PST" -> 4 — the required size, not an offset in seconds)
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _get_tzname,
                 size_t *retval,
                 char *buf,
                 size_t size,
                 int index)
  {
    if (!buf || !retval || (unsigned)index > 1) {
      _set_errno(EINVAL);
      return EINVAL;
    }
    const musl::tz::state &st = musl::tz::get();
    const char *name = index ? st.dlt_name : st.std_name;
    size_t need = strlen(name) + 1;
    if (size < need) {
      if (size)
        buf[0] = 0;
      return ERANGE;
    }
    memcpy(buf, name, need);
    *retval = need;
    return 0;
  }
} // namespace mingw_thunk
