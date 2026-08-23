#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(advapi32,
                 24,
                 LSTATUS,
                 APIENTRY,
                 RegQueryValueExA,
                 _In_ HKEY hKey,
                 _In_opt_ LPCSTR lpValueName,
                 _Reserved_ LPDWORD lpReserved,
                 _Out_opt_ LPDWORD lpType,
                 _Out_writes_bytes_to_opt_(*lpcbData, *lpcbData) LPBYTE lpData,
                 _When_(lpData == NULL, _Out_opt_)
                     _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData)
  {
    d::w_str w_name;
    const wchar_t *name = nullptr;
    if (lpValueName) {
      if (!w_name.from_u(lpValueName))
        return ERROR_OUTOFMEMORY;
      name = w_name.c_str();
    }

    DWORD type = 0;
    DWORD size_w = 0;
    LSTATUS status =
        RegQueryValueExW(hKey, name, lpReserved, &type, nullptr, &size_w);

    if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA)
      return status;

    if (lpType)
      *lpType = type;

    if (type != REG_SZ && type != REG_EXPAND_SZ && type != REG_MULTI_SZ) {
      // byte semantics
      return RegQueryValueExW(hKey, name, lpReserved, lpType, lpData, lpcbData);
    }

    if (size_w % sizeof(WCHAR))
      size_w += sizeof(WCHAR) - size_w % sizeof(WCHAR);

    d::w_str w_data;
    if (!w_data.resize(size_w / sizeof(WCHAR)))
      return ERROR_OUTOFMEMORY;

    DWORD size_w2 = size_w;
    status = RegQueryValueExW(
        hKey, name, lpReserved, nullptr, (LPBYTE)w_data.data(), &size_w2);
    if (status != ERROR_SUCCESS)
      return status;

    const size_t len_w = size_w2 / sizeof(WCHAR);

    d::u_str out;
    if (type == REG_MULTI_SZ) {
      const wchar_t *cur = w_data.data();
      const wchar_t *end = cur + len_w;
      while (cur < end) {
        size_t seg = c::wcsnlen(cur, end - cur);
        if (seg == 0)
          break;
        if (seg > size_t(INT_MAX))
          return ERROR_NOT_ENOUGH_MEMORY;
        int need = d::u_str::size_from_w(cur, int(seg));
        if (need < 0)
          return ERROR_INVALID_DATA;
        size_t old = out.size();
        if (!out.resize(old + need + 1))
          return ERROR_OUTOFMEMORY;
        if (need)
          d::u_str::fixed_buffer_from_w(out.data() + old, need, cur, int(seg));
        cur += seg + 1;
      }
      if (!out.push_back('\0'))
        return ERROR_OUTOFMEMORY;
    } else {
      if (len_w > size_t(INT_MAX))
        return ERROR_NOT_ENOUGH_MEMORY;
      if (!out.from_w(w_data.data(), int(len_w)))
        return ERROR_INVALID_DATA;
    }

    const DWORD need = DWORD(out.size());

    if (!lpData) {
      *lpcbData = need;
      return ERROR_SUCCESS;
    }

    if (need > *lpcbData) {
      *lpcbData = need;
      return ERROR_MORE_DATA;
    }

    if (need)
      c::memcpy(lpData, out.data(), need);
    *lpcbData = need;
    return ERROR_SUCCESS;
  }
} // namespace mingw_thunk
