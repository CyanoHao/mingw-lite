#include <thunk/_common.h>
#include <thunk/_crt.h>

namespace mingw_thunk
{
  __DECLARE_CRT_IMP_ALIAS(open, _open)
}
