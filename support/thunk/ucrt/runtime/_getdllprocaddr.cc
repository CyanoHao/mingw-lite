#include <thunk/_common.h>
#include <thunk/string.h>

#include <stdint.h>

namespace mingw_thunk
{
  // The reference signature (exec/getproc.cpp):
  //   typedef int (__cdecl* proc_address_type)();
  typedef int (__cdecl *proc_address_t)();

  namespace i
  {
    namespace dll
    {
      // mingw only declares the ANSI GetProcAddress, so the wide one is
      // resolved once out of kernel32 -- the ASCII export name goes
      // through the ANSI entry, exactly as the module_handle helper does.
      using get_proc_w_t = FARPROC(WINAPI *)(HMODULE, LPCWSTR);

      inline get_proc_w_t get_proc_w() noexcept
      {
        static auto *pfn = reinterpret_cast<get_proc_w_t>(
            GetProcAddress(internal::module_kernel32().module, "GetProcAddressW"));
        return pfn;
      }
    } // namespace dll
  } // namespace i

  // exec/getproc.cpp, adapted.  The shell only ever sees UTF-8 narrow
  // strings, so the procedure name is transcoded and looked up with the
  // wide entry; the ordinal form follows the reference's shape (a null
  // name plus an ordinal of at most 65535) and truncates through
  // MAKEINTRESOURCEA, so a negative ordinal can never reach
  // GetProcAddress as a string pointer.
  //
  // wine anchors: a name with ordinal -1 resolves, a name with any other
  // ordinal returns null, ordinal 0 and ordinal 70000 return null, and an
  // invalid module handle returns null rather than faulting.
  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 proc_address_t,
                 __cdecl,
                 _getdllprocaddr,
                 intptr_t module_handle,
                 char *procedure_name,
                 intptr_t ordinal)
  {
    HMODULE module = reinterpret_cast<HMODULE>(module_handle);
    auto get_proc_w = i::dll::get_proc_w();

    if (procedure_name == nullptr) {
      if (ordinal > 65535)
        return nullptr;
      /* MAKEINTRESOURCEW truncates to a WORD in the low half with a zero
       * high half, which is the only shape GetProcAddress reads as an
       * ordinal; the reference's untruncated cast would hand a negative
       * ordinal over as a string pointer */
      return get_proc_w
                 ? reinterpret_cast<proc_address_t>(
                       get_proc_w(module, MAKEINTRESOURCEW(ordinal)))
                 : nullptr;
    }

    if (ordinal != -1)
      return nullptr;

    d::w_str w_name;
    if (!w_name.from_u(procedure_name))
      return nullptr;

    if (get_proc_w)
      return reinterpret_cast<proc_address_t>(get_proc_w(module, w_name.c_str()));

    // no wide entry: the export name is ASCII in practice, so the ANSI
    // form is still usable
    return reinterpret_cast<proc_address_t>(GetProcAddress(module, procedure_name));
  }
} // namespace mingw_thunk
