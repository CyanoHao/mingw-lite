#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  namespace i
  {
    char **pgmptr_slot(); // ucrt/runtime/__p__pgmptr.cc
  } // namespace i

  __DEFINE_THUNK(
      api_ms_win_crt_runtime_l1_1_0, 0, errno_t, __cdecl, _get_pgmptr, char **value)
  {
    // wine anchor (M7): null out-pointer -> EINVAL, no crash
    if (!value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    *value = *i::pgmptr_slot();
    return 0;
  }
} // namespace mingw_thunk
