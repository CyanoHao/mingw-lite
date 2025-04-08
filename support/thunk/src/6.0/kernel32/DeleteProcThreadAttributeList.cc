#include <thunk/_common.h>

#include <processthreadsapi.h>
#include <windows.h>

namespace mingw_thunk
{
  // YY-Thunks 1.1.6
  // Minimum supported client	Windows Vista [desktop apps only]
  // Minimum supported server	Windows Server 2008 [desktop apps only]
  __DEFINE_THUNK(kernel32,
                 4,
                 VOID,
                 WINAPI,
                 DeleteProcThreadAttributeList,
                 _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList)
  {
    if (const auto pDeleteProcThreadAttributeList =
            try_get_DeleteProcThreadAttributeList()) {
      return pDeleteProcThreadAttributeList(lpAttributeList);
    }

    // Vista原版什么也没有做……
  }
} // namespace mingw_thunk
