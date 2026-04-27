#include <iconv.h>

#include <errno.h>
#include <stdlib.h>

#include "descriptor.h"
#include "encoding.h"

namespace libiconv
{
  extern "C" iconv_t iconv_open(const char *tocode, const char *fromcode)
  {
    if (!tocode || !fromcode) {
      errno = EINVAL;
      return (iconv_t)-1;
    }

    resolve_result to_res = resolve_encoding(tocode);
    if (to_res.cp == unsigned(-1)) {
      errno = EINVAL;
      return (iconv_t)-1;
    }

    resolve_result from_res = resolve_encoding(fromcode);
    if (from_res.cp == unsigned(-1)) {
      errno = EINVAL;
      return (iconv_t)-1;
    }

    auto cd = (iconv_descriptor *)calloc(1, sizeof(iconv_descriptor));
    if (!cd) {
      errno = ENOMEM;
      return (iconv_t)-1;
    }

    cd->from_cp = from_res.cp;
    cd->to_cp = to_res.cp;
    cd->discard_ilseq = to_res.discard_ilseq || from_res.discard_ilseq;
    cd->translit = to_res.translit || from_res.translit;
    cd->from_pending_len = 0;
    cd->pending_high_surrogate = 0;

    return cd;
  }
} // namespace libiconv
