#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  namespace
  {
    // Resolves _wsearchenv_s from msvcrt.dll at runtime (null when absent,
    // e.g. on XP). The __DEFINE_THUNK below only generates a resolver for the
    // narrow _searchenv_s name, so the wide original is resolved separately.
    // An explicit signature is used because _wsearchenv_s has a C++ template
    // overload that makes decltype(::_wsearchenv_s) ambiguous.
    inline auto *resolve__wsearchenv_s() noexcept
    {
      using fn_t =
          errno_t __cdecl(const wchar_t *, const wchar_t *, wchar_t *, size_t);
      static auto *pfn =
          internal::module_msvcrt().get_function<fn_t>("_wsearchenv_s");
      return pfn;
    }
  } // namespace

  __DEFINE_THUNK(msvcrt,
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

    const wchar_t *file_arg = filename ? wfile.c_str() : nullptr;
    const wchar_t *env_arg = envvar ? wenv.c_str() : nullptr;

    d::w_str wresult;

#if THUNK_LEVEL >= NTDDI_VISTA

    if (!wresult.resize(result_count)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    errno_t status = __ms__wsearchenv_s(
        file_arg, env_arg, wresult.data(), result_count);

    if (status != 0)
      return status;

#else

    if (auto *pfn = resolve__wsearchenv_s()) {
      if (!wresult.resize(result_count)) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }

      errno_t status = pfn(file_arg, env_arg, wresult.data(), result_count);

      if (status != 0)
        return status;
    } else {
      // Fallback: _wsearchenv (void, writes up to _MAX_PATH). Probe with a
      // full _MAX_PATH wide buffer, then size-check after UTF-8 conversion.
      if (!wresult.resize(_MAX_PATH)) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }

      _wsearchenv(file_arg, env_arg, wresult.data());

      if (wresult.data()[0] == 0) {
        _set_errno(ENOENT);
        return ENOENT;
      }
    }

#endif

    // Shared: convert the wide result back to UTF-8 and size-check. A wide
    // path may fit in result_count wchars yet expand beyond result_count bytes.
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
