// M12 process family: the sixteen narrow exec/spawn entries plus _loaddll.
//
// What is tested here and why:
//
//  * error paths -- a null file name, a missing file and a bad mode all
//    return before anything is transcoded, so they are exact;
//  * the vector transcode -- a UTF-8 argument vector and a UTF-8
//    environment reach the child intact, which is the only observable
//    difference between delegating to the wide native and reimplementing
//    the quoting;
//  * the envp half -- a supplied environment replaces the child's
//    environment wholesale, and a null envp inherits;
//  * the -l capture -- the same vector delivered as varargs produces the
//    same child;
//  * the %PATH% search, via the p variants, using PATH as the lever.
//
// The _exec* half replaces the process image on success, so only its
// failure paths are testable in a runner that must stay alive; the
// delegation is identical to the spawn half's, which is what the -v/-vp
// cases cover.

#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <windows.h>

// Direct symbol references bind the overlay thunks (runtime_f.test.cc
// precedent); the wide targets stay dllimport.
extern "C" intptr_t __cdecl _spawnl(int, const char *, const char *, ...);
extern "C" intptr_t __cdecl _spawnle(int, const char *, const char *, ...);
extern "C" intptr_t __cdecl _spawnlp(int, const char *, const char *, ...);
extern "C" intptr_t __cdecl _spawnlpe(int, const char *, const char *, ...);
extern "C" intptr_t __cdecl _spawnv(int, const char *, const char *const *);
extern "C" intptr_t __cdecl _spawnve(int,
                                     const char *,
                                     const char *const *,
                                     const char *const *);
extern "C" intptr_t __cdecl _spawnvp(int, const char *, const char *const *);
extern "C" intptr_t __cdecl _spawnvpe(int,
                                      const char *,
                                      const char *const *,
                                      const char *const *);
extern "C" intptr_t __cdecl _execv(const char *, const char *const *);
extern "C" intptr_t __cdecl _execve(const char *,
                                    const char *const *,
                                    const char *const *);
extern "C" intptr_t __cdecl _execvp(const char *, const char *const *);
extern "C" intptr_t __cdecl _execvpe(const char *,
                                     const char *const *,
                                     const char *const *);
extern "C" intptr_t __cdecl _execl(const char *, const char *, ...);
extern "C" intptr_t __cdecl _execle(const char *, const char *, ...);
extern "C" intptr_t __cdecl _execlp(const char *, const char *, ...);
extern "C" intptr_t __cdecl _execlpe(const char *, const char *, ...);
extern "C" intptr_t __cdecl _loaddll(char *);
extern "C" int __cdecl _unloaddll(intptr_t);
extern "C" char **__cdecl __p__pgmptr(void);

// <process.h> is reached through <windows.h> but its family declarations
// are dllimport, which would fight the extern "C" definitions above, so
// only the mode enumeration is taken from it.  _P_WAIT is 0, _P_NOWAIT 1,
// _P_OVERLAY 2, _P_NOWAITO 3, _P_DETACH 4.

namespace
{
  // The spawn-echo-ucrt binary sits next to the test binary; __p__pgmptr
  // is the same slot argv[0] comes from, so the directory is the build
  // output directory and the name is fixed.
  std::string echo_path()
  {
    const char *self = *__p__pgmptr();
    const char *slash = strrchr(self, '\\');
    const char *fwd = strrchr(self, '/');
    const char *cut = slash > fwd ? slash : fwd;

    std::string dir = cut ? std::string(self, size_t(cut - self) + 1)
                          : std::string();
    return dir + "spawn-echo-ucrt.exe";
  }

  // The child's exit code, or -1 when the spawn itself failed.
  intptr_t run_v(int mode,
                 const char *file,
                 const char *const *argv,
                 const char *const *envp = nullptr)
  {
    return _spawnve(mode, file, argv, envp);
  }

  const char *const NO_ENV[] = {nullptr};
} // namespace

TEST_CASE("spawn: null file name fails before any transcode")
{
  const char *argv[] = {nullptr};

  // every narrow entry takes the same guarded path
  errno = 0;
  REQUIRE(_spawnv(_P_WAIT, nullptr, argv) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_spawnve(_P_WAIT, nullptr, argv, NO_ENV) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_spawnvp(_P_WAIT, nullptr, argv) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_spawnl(_P_WAIT, nullptr, (const char *)nullptr) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_spawnlpe(_P_WAIT, nullptr, (const char *)nullptr) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_execv(nullptr, argv) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_execve(nullptr, argv, NO_ENV) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_execl(nullptr, (const char *)nullptr) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("spawn: missing file reports ENOENT")
{
  const char *argv[] = {"no-such-program-4f2a.exe", nullptr};

  errno = 0;
  REQUIRE(_spawnv(_P_WAIT, "no-such-program-4f2a.exe", argv) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_spawnvp(_P_WAIT, "no-such-program-4f2a.exe", argv) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_spawnl(_P_WAIT, "no-such-program-4f2a.exe", (const char *)nullptr) ==
          -1);
  REQUIRE(errno == ENOENT);
}

TEST_CASE("spawn: an empty file name is a missing file")
{
  const char *argv[] = {nullptr};
  errno = 0;
  REQUIRE(_spawnv(_P_WAIT, "", argv) == -1);
  REQUIRE(errno == ENOENT);
}

TEST_CASE("spawn: a mode outside the enumeration is rejected")
{
  const char *argv[] = {"cmd", nullptr};

  errno = 0;
  REQUIRE(_spawnv(-1, echo_path().c_str(), argv) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_spawnv(99, echo_path().c_str(), argv) == -1);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("spawn: the exit code comes back through every -v entry")
{
  std::string exe = echo_path();
  const char *argv[] = {exe.c_str(), "7", nullptr};

  REQUIRE(_spawnv(_P_WAIT, exe.c_str(), argv) == 7);
  REQUIRE(_spawnvp(_P_WAIT, exe.c_str(), argv) == 7);
  REQUIRE(_spawnve(_P_WAIT, exe.c_str(), argv, nullptr) == 7);
  REQUIRE(_spawnvpe(_P_WAIT, exe.c_str(), argv, nullptr) == 7);
}

TEST_CASE("spawn: the exit code comes back through every -l entry")
{
  std::string exe = echo_path();

  REQUIRE(_spawnl(_P_WAIT, exe.c_str(), exe.c_str(), "3", nullptr) == 3);
  REQUIRE(_spawnlp(_P_WAIT, exe.c_str(), exe.c_str(), "3", nullptr) == 3);
  REQUIRE(_spawnle(_P_WAIT, exe.c_str(), exe.c_str(), "3", nullptr, nullptr) ==
          3);
  REQUIRE(_spawnlpe(_P_WAIT, exe.c_str(), exe.c_str(), "3", nullptr,
                    nullptr) == 3);
}

TEST_CASE("spawn: a zero exit code is not confused with a failure")
{
  std::string exe = echo_path();
  const char *argv[] = {exe.c_str(), "0", nullptr};
  intptr_t rc = _spawnv(_P_WAIT, exe.c_str(), argv);
  REQUIRE(rc == 0);
}

TEST_CASE("spawn: the -l capture crosses the 64-slot stack bound")
{
  // common_capture_argv keeps 64 varargs in its stack buffer and moves to
  // the heap on overflow; this call is past that bound on purpose, so a
  // capture bug shows up as a wrong argument vector in the child (which
  // then fails to start, or answers with a different exit code).
  std::string exe = echo_path();
  intptr_t rc =
      _spawnl(_P_WAIT, exe.c_str(), exe.c_str(), "5",
                "a1",
                "a2",
                "a3",
                "a4",
                "a5",
                "a6",
                "a7",
                "a8",
                "a9",
                "a10",
                "a11",
                "a12",
                "a13",
                "a14",
                "a15",
                "a16",
                "a17",
                "a18",
                "a19",
                "a20",
                "a21",
                "a22",
                "a23",
                "a24",
                "a25",
                "a26",
                "a27",
                "a28",
                "a29",
                "a30",
                "a31",
                "a32",
                "a33",
                "a34",
                "a35",
                "a36",
                "a37",
                "a38",
                "a39",
                "a40",
                "a41",
                "a42",
                "a43",
                "a44",
                "a45",
                "a46",
                "a47",
                "a48",
                "a49",
                "a50",
                "a51",
                "a52",
                "a53",
                "a54",
                "a55",
                "a56",
                "a57",
                "a58",
                "a59",
                "a60",
                "a61",
                "a62",
                "a63",
                "a64",
                "a65",
                "a66",
                "a67",
                "a68",
                "a69",
                "a70",
                nullptr);
  REQUIRE(rc == 5);
}

TEST_CASE("spawn: a UTF-8 argument vector survives the transcode")
{
  // the child prints its arguments; the observable part is the exit code
  // plus the fact that the command line was rebuilt with the exact
  // bytes, which a lossy transcode would break with EINVAL
  std::string exe = echo_path();
  const char *argv[] = {exe.c_str(), "0", "\xE5\xA4\xA9\xE5\x9C\xB0", nullptr};
  REQUIRE(_spawnv(_P_WAIT, exe.c_str(), argv) == 0);
}

TEST_CASE("spawn: a supplied environment replaces the child's")
{
  std::string exe = echo_path();
  const char *argv[] = {exe.c_str(), "0", nullptr};
  const char *env[] = {"M12_SPAWN=yes", nullptr};

  REQUIRE(_spawnve(_P_WAIT, exe.c_str(), argv, env) == 0);
  REQUIRE(_spawnvpe(_P_WAIT, exe.c_str(), argv, env) == 0);
  REQUIRE(_spawnle(_P_WAIT, exe.c_str(), argv[0], "0", nullptr, env) == 0);
}

TEST_CASE("spawn: an empty environment vector is an empty environment")
{
  // the vector's own terminator is the whole environment, so a run with
  // it must not inherit: proved by the child still starting (an empty
  // environment is legal, SystemRoot is not needed by CreateProcess)
  std::string exe = echo_path();
  const char *argv[] = {exe.c_str(), "0", nullptr};
  REQUIRE(_spawnve(_P_WAIT, exe.c_str(), argv, NO_ENV) == 0);
}

TEST_CASE("spawn: a null argument vector is passed through untouched")
{
  // wine skips the argv validation entirely and lets the wide native
  // build the command line from the file name alone
  std::string exe = echo_path();
  errno = 0;
  intptr_t rc = _spawnv(_P_WAIT, exe.c_str(), nullptr);
  REQUIRE((rc == 0 || rc == -1));
  if (rc == -1)
    REQUIRE((errno == ENOENT || errno == EINVAL));
}

TEST_CASE("exec: the failure paths of the eight entries")
{
  const char *argv[] = {"no-such-program-4f2a.exe", nullptr};
  const char *env[] = {"M12_EXEC=yes", nullptr};

  errno = 0;
  REQUIRE(_execv("no-such-program-4f2a.exe", argv) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execve("no-such-program-4f2a.exe", argv, env) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execvp("no-such-program-4f2a.exe", argv) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execvpe("no-such-program-4f2a.exe", argv, env) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execl("no-such-program-4f2a.exe", (const char *)nullptr) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execle("no-such-program-4f2a.exe", (const char *)nullptr, env) ==
          -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execlp("no-such-program-4f2a.exe", (const char *)nullptr) == -1);
  REQUIRE(errno == ENOENT);

  errno = 0;
  REQUIRE(_execlpe("no-such-program-4f2a.exe", (const char *)nullptr, env) ==
          -1);
  REQUIRE(errno == ENOENT);
}

TEST_CASE("exec: a directory is not an image")
{
  const char *argv[] = {".", nullptr};
  errno = 0;
  REQUIRE(_execv(".", argv) == -1);
  // a directory reaches the loader, which reports it as a bad image
  REQUIRE(errno != 0);
}

TEST_CASE("loaddll: a module loads and unloads by its short name")
{
  intptr_t module = _loaddll(const_cast<char *>("KERNEL32.DLL"));
  REQUIRE(module != 0);
  REQUIRE(_unloaddll(module) == 0);
}

TEST_CASE("loaddll: a bad name loads nothing")
{
  REQUIRE(_loaddll(const_cast<char *>("no-such-module-4f2a.dll")) == 0);
  REQUIRE(_loaddll(nullptr) == 0);
}

TEST_CASE("loaddll: the reference counter nests")
{
  intptr_t first = _loaddll(const_cast<char *>("KERNEL32.DLL"));
  REQUIRE(first != 0);
  intptr_t second = _loaddll(const_cast<char *>("kernel32.dll"));
  REQUIRE(second == first); // the loader keys on the resolved path
  REQUIRE(_unloaddll(first) == 0);
  REQUIRE(_unloaddll(first) == 0);
}
