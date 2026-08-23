// Spawn target for the M12 process tests.
//
// The `_exec*` half of the family replaces the process image, so its
// success path is only observable by this program's own output; the
// `_spawn*` half is observed through the exit code.  Keeping the target
// separate from the Catch2 binary also keeps the two link orders apart:
// this one must not pull in the engine.
//
// argv[1] is the exit code (default 0); everything after it is echoed
// one argument per line, which is what makes the vector transcode
// observable end to end.

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int code = 0;
  int first = 1;

  if (argc > 1) {
    code = atoi(argv[1]);
    first = 2;
  }

  for (int i = first; i < argc; i++)
    printf("arg[%d]=%s\n", i - first, argv[i]);

  fflush(stdout);
  return code;
}
