#include "_common.h"

#include "string.h"

#include <tlhelp32.h>

namespace mingw_thunk::i
{
  inline MODULEENTRY32W a2w(const MODULEENTRY32 &narrow)
  {
    MODULEENTRY32W res{};

    res.th32ModuleID = narrow.th32ModuleID;
    res.th32ProcessID = narrow.th32ProcessID;
    res.GlblcntUsage = narrow.GlblcntUsage;
    res.ProccntUsage = narrow.ProccntUsage;
    res.modBaseAddr = narrow.modBaseAddr;
    res.modBaseSize = narrow.modBaseSize;
    res.hModule = narrow.hModule;
    d::w_str::best_effort_from_a(
        res.szModule, MAX_MODULE_NAME32 + 1, narrow.szModule);
    d::w_str::best_effort_from_a(res.szExePath, MAX_PATH, narrow.szExePath);

    return res;
  }

  inline MODULEENTRY32 w2u(const MODULEENTRY32W &wide)
  {
    MODULEENTRY32 res{};

    res.th32ModuleID = wide.th32ModuleID;
    res.th32ProcessID = wide.th32ProcessID;
    res.GlblcntUsage = wide.GlblcntUsage;
    res.ProccntUsage = wide.ProccntUsage;
    res.modBaseAddr = wide.modBaseAddr;
    res.modBaseSize = wide.modBaseSize;
    res.hModule = wide.hModule;
    d::u_str::best_effort_from_w(
        res.szModule, MAX_MODULE_NAME32 + 1, wide.szModule);
    d::u_str::best_effort_from_w(res.szExePath, MAX_PATH, wide.szExePath);

    return res;
  }

  inline MODULEENTRY32 w2a(const MODULEENTRY32W &wide)
  {
    MODULEENTRY32 res{};

    res.th32ModuleID = wide.th32ModuleID;
    res.th32ProcessID = wide.th32ProcessID;
    res.GlblcntUsage = wide.GlblcntUsage;
    res.ProccntUsage = wide.ProccntUsage;
    res.modBaseAddr = wide.modBaseAddr;
    res.modBaseSize = wide.modBaseSize;
    res.hModule = wide.hModule;
    d::a_str::best_effort_from_w(
        res.szModule, MAX_MODULE_NAME32 + 1, wide.szModule);
    d::a_str::best_effort_from_w(res.szExePath, MAX_PATH, wide.szExePath);

    return res;
  }
} // namespace mingw_thunk::i
