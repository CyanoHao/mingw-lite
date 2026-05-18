#pragma once

#include <windows.h>

namespace mingw_thunk::tls_fix
{
  void fix_pre(DWORD reason, LPVOID reserved);
  void fix_post(DWORD reason, LPVOID reserved);
}
