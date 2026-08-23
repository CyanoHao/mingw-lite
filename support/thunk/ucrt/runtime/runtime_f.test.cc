#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Direct symbol references bind the overlay thunks (fopen.test.cc
// precedent); int matches errno_t on the ABI level.
extern "C" char **__cdecl __p__pgmptr(void);
extern "C" int __cdecl _get_pgmptr(char **value);
extern "C" char **__cdecl _get_initial_narrow_environment(void);
extern "C" char *_get_narrow_winmain_command_line(void);

static bool env_has(char **env, const char *name)
{
  size_t n = strlen(name);
  for (char **e = env; e && *e; e++)
    if (strncmp(*e, name, n) == 0 && (*e)[n] == '=')
      return true;
  return false;
}

TEST_CASE("pgmptr")
{
  char **slot = __p__pgmptr();
  REQUIRE(slot != nullptr);
  REQUIRE(*slot != nullptr);
  REQUIRE(strlen(*slot) > 0);

  // stable slot: the char* is cached, the address never moves
  char **slot2 = __p__pgmptr();
  REQUIRE(slot2 == slot);
}

TEST_CASE("get_pgmptr")
{
  char *value = nullptr;
  errno = 0;
  REQUIRE(_get_pgmptr(&value) == 0);
  REQUIRE(value != nullptr);
  REQUIRE(value == *__p__pgmptr());

  errno = 0;
  REQUIRE(_get_pgmptr(nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("get_initial_narrow_environment")
{
  char **initial = _get_initial_narrow_environment();
  REQUIRE(initial != nullptr);
  REQUIRE(initial[0] != nullptr);

  // frozen snapshot (wine anchor): _putenv afterwards never shows up
  // in the initial block, while the live getenv sees it
  REQUIRE(_putenv("U8CRT_M7_VAR=42") == 0);
  REQUIRE(getenv("U8CRT_M7_VAR") != nullptr);
  REQUIRE_FALSE(env_has(initial, "U8CRT_M7_VAR"));

  char **again = _get_initial_narrow_environment();
  REQUIRE(again == initial); // same cached block

  _putenv("U8CRT_M7_VAR="); // remove again
  REQUIRE(getenv("U8CRT_M7_VAR") == nullptr);
}

TEST_CASE("get_narrow_winmain_command_line")
{
  // WinMain semantics: the part after the program token.  Headless
  // runs without arguments anchor to the empty string; only the
  // non-null shape is asserted here (the harness may pass flags).
  char *cl = _get_narrow_winmain_command_line();
  REQUIRE(cl != nullptr);
  char *cl2 = _get_narrow_winmain_command_line();
  REQUIRE(cl2 == cl);
}
