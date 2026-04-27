#include "encoding.h"

#include <ctype.h>

#include "name_table.h"

namespace libiconv
{
  resolve_result resolve_encoding(const char *code)
  {
    unsigned cp = -1;
    bool translit = false;
    bool discard_ilseq = false;

    constexpr size_t buf_size = 64 + 9 + 9 + 1;
    char buf[buf_size];
    size_t len = 0;

    while (len < buf_size - 1) {
      unsigned char c = (unsigned char)code[len];
      if (c == '\0')
        break;
      c = tolower(c);
      buf[len++] = c;
    }
    buf[len] = '\0';

    static constexpr string_view suffix_translit("//translit");
    static constexpr string_view suffix_ignore("//ignore");

    string_view text(buf, len);
    for (;;) {
      if (text.size() >= suffix_translit.size() &&
          text.substr(text.size() - suffix_translit.size()) ==
              suffix_translit) {
        text = text.substr(0, text.size() - suffix_translit.size());
        translit = true;
      } else if (text.size() >= suffix_ignore.size() &&
                 text.substr(text.size() - suffix_ignore.size()) ==
                     suffix_ignore) {
        text = text.substr(0, text.size() - suffix_ignore.size());
        discard_ilseq = true;
      } else {
        break;
      }
    }

    if (text.empty())
      return {cp, translit, discard_ilseq};

    for (auto it = name_table_begin; it != name_table_end; ++it)
      if (it->name == text)
        return {it->cp, translit, discard_ilseq};

    return {cp, translit, discard_ilseq};
  }
} // namespace libiconv
