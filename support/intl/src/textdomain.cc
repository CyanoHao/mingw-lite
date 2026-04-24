#include <libintl.h>

#include "internal-state.h"

namespace intl
{
  extern "C" char *textdomain(const char *domainname)
  {
    if (!domainname)
      return default_domain.data();

    default_domain = domainname;
    return default_domain.data();
  }
} // namespace intl
