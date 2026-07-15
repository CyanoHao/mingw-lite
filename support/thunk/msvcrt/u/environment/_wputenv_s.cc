#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>
#include <thunk/utf8-musl.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 errno_t,
                 __cdecl,
                 _wputenv_s,
                 const wchar_t *name,
                 const wchar_t *value)
  {
    if (!name || !value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    d::u_str key;
    if (!key.from_w(name)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    int result;
    if (value[0] == 0)
      result = musl::unsetenv(key.c_str(), key.size());
    else {
      d::u_str val;
      if (!val.from_w(value)) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }
      result =
          musl::setenv(key.c_str(), key.size(), val.c_str(), val.size(), 1);
    }

    if (result != 0)
      return errno;

#if THUNK_LEVEL >= NTDDI_VISTA

    __ms__wputenv_s(name, value);

#else

    if (auto *pfn = try_get__wputenv_s())
      pfn(name, value);
    else {
      size_t name_len = wcslen(name);
      size_t value_len = wcslen(value);

      d::w_str envstring;
      if (!envstring.resize(name_len + 1 + value_len)) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }

      wmemcpy(envstring.data(), name, name_len);
      envstring[name_len] = L'=';
      wmemcpy(envstring.data() + name_len + 1, value, value_len);

      __ms__wputenv(envstring.c_str());
    }

#endif

    return 0;
  }
} // namespace mingw_thunk
