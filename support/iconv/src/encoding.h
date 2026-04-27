#pragma once

namespace libiconv
{
  struct resolve_result
  {
    unsigned int cp;
    bool translit;
    bool discard_ilseq;
  };

  resolve_result resolve_encoding(const char *code);
} // namespace libiconv
