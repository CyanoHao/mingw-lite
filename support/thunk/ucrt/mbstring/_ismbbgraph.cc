#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // printable and not a space (plan-3 D8i/D8j).  Answers 0/1 rather than a
  // ctype mask (D8e).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbbgraph,
                 unsigned int c)
  {
    return mbstring::ismbb(c, mbstring::B_GRAPH);
  }
} // namespace mingw_thunk
