#pragma once

#include "corecrt_internal.h"

#define _FILL_STRING _SECURECRT__FILL_STRING

#define _RESET_STRING(_String, _Size)                                          \
  *(_String) = 0;                                                              \
  _FILL_STRING((_String), (_Size), 1);

#define _ASSIGN_IF_NOT_NULL(_Pointer, _Value)                                  \
  if ((_Pointer) != NULL)                                                      \
  {                                                                            \
    *(_Pointer) = (_Value);                                                    \
  }
