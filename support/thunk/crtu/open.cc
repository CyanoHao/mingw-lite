#include "thunk/_crt.h"
#include <thunk/_common.h>

namespace mingw_thunk
{
  __DECLARE_CRT_IMP_ALIAS(open, _open)
}
