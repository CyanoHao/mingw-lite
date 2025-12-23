#include <thunk/_crt.h>
#include <thunk/string.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(FILE *, fopen, const char *path, const char *mode)
  {
    stl::wstring w_path = internal::u2w(path);
    stl::wstring w_mode;
    while (*mode) {
      switch (*mode) {
      case 'r':
      case 'w':
      case 'a':
      case '+':
        w_mode.push_back(*mode);
        break;
      case 't':
      case 'b':
        // ignore (add 'b' later)
        break;
      case 'x':
      case 'c':
      case 'n':
      case 'N':
      case 'S':
      case 'R':
      case 'T':
      case 'D':
        w_mode.push_back(*mode);
        break;
      case ',':
        // ,ccs=UTF-8
        goto stop;
      }
    }
  stop:
    w_mode.push_back('b');
    return _wfopen(w_path.c_str(), w_mode.c_str());
  }
} // namespace mingw_thunk
