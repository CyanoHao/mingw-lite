#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _searchenv_s,
                 const char *filename,
                 const char *envvar,
                 char *result,
                 size_t result_count)
  {
    if (!result || result_count == 0) {
      _set_errno(EINVAL);
      return EINVAL;
    }
    result[0] = 0;

    d::w_str wfile;
    if (filename && !wfile.from_u(filename)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    d::w_str wenv;
    if (envvar && !wenv.from_u(envvar)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    d::w_str wresult;
    if (!wresult.resize(result_count)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    errno_t status = _wsearchenv_s(filename ? wfile.c_str() : nullptr,
                                   envvar ? wenv.c_str() : nullptr,
                                   wresult.data(),
                                   result_count);

    if (status != 0)
      return status;

    d::u_str ures;
    if (!ures.from_w(wresult.data())) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    if (ures.size() + 1 > result_count) {
      _set_errno(ERANGE);
      return ERANGE;
    }

    memcpy(result, ures.c_str(), ures.size());
    result[ures.size()] = 0;
    return 0;
  }
} // namespace mingw_thunk
