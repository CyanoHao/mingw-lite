#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <corecrt_startup.h>
#include <stdlib.h>

#include <unistd.h>
#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    // Defined in the regular archive set
    // (ucrt/runtime/_get_initial_narrow_environment.cc) so that links
    // which never pull this startup-dep member still resolve.
    char **u8envp_from_wenvp(wchar_t **wenvp);
    char **frozen_envp_copy(char **envp);
    extern char **g_initial_u8envp;
  } // namespace i

  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _initialize_narrow_environment)
  {
    // BEWARE: runtime is not fully initialized yet!
    _initialize_wide_environment();
    musl::__environ = i::u8envp_from_wenvp(*__p__wenviron());
    i::g_initial_u8envp = i::frozen_envp_copy(musl::__environ);
    return 0;
  }
} // namespace mingw_thunk
