// M12 _getdllprocaddr (plus the _loaddll half it consumes).
//
// The face resolves through the wide GetProcAddress, which mingw does
// not declare, so the wide entry is looked up out of kernel32 once.  The
// cases below pin the three-way contract the reference documents --
// by name with ordinal -1, by ordinal with a null name, and nothing
// else -- plus the ordinal range, the bad-handle path and the hardening
// that keeps a negative ordinal from ever reaching GetProcAddress as a
// string pointer.

#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

// Direct symbol references bind the overlay thunks (runtime_f.test.cc
// precedent).
typedef int (__cdecl *proc_address_t)();
extern "C" proc_address_t __cdecl _getdllprocaddr(intptr_t, char *, intptr_t);
extern "C" intptr_t __cdecl _loaddll(char *);

namespace
{
  // A module that is loaded in every process and exports both a name and
  // a handful of ordinals.
  intptr_t kernel()
  {
    static intptr_t handle =
        _loaddll(const_cast<char *>("KERNEL32.DLL"));
    return handle;
  }
} // namespace

TEST_CASE("getdllprocaddr: a name with ordinal -1 resolves")
{
  REQUIRE(kernel() != 0);

  proc_address_t by_name =
      _getdllprocaddr(kernel(), const_cast<char *>("GetTickCount"), -1);
  REQUIRE(by_name != nullptr);

  // the resolved pointer really is the export
  FARPROC direct = GetProcAddress(reinterpret_cast<HMODULE>(kernel()),
                                  "GetTickCount");
  REQUIRE(reinterpret_cast<void *>(by_name) == reinterpret_cast<void *>(direct));
}

TEST_CASE("getdllprocaddr: a name with any other ordinal is refused")
{
  REQUIRE(kernel() != 0);

  char *name = const_cast<char *>("GetTickCount");
  REQUIRE(_getdllprocaddr(kernel(), name, 0) == nullptr);
  REQUIRE(_getdllprocaddr(kernel(), name, 1) == nullptr);
  REQUIRE(_getdllprocaddr(kernel(), name, 42) == nullptr);
}

TEST_CASE("getdllprocaddr: an unknown name is null")
{
  REQUIRE(kernel() != 0);

  REQUIRE(_getdllprocaddr(kernel(),
                          const_cast<char *>("NoSuchExport4f2a"), -1) ==
          nullptr);
  // an empty name is an unknown name
  REQUIRE(_getdllprocaddr(kernel(), const_cast<char *>(""), -1) == nullptr);
}

TEST_CASE("getdllprocaddr: the ordinal form walks MAKEINTRESOURCE")
{
  REQUIRE(kernel() != 0);

  // kernel32 exports by ordinal; the low 65536 must be passed through and
  // anything above it refused
  REQUIRE(_getdllprocaddr(kernel(), nullptr, 70000) == nullptr);
  REQUIRE(_getdllprocaddr(kernel(), nullptr, 0x10000) == nullptr);
  REQUIRE(_getdllprocaddr(kernel(), nullptr, -1) == nullptr);

  // ordinal 0 is not a callable export, and the truncating cast keeps a
  // negative ordinal from ever reaching GetProcAddress as a pointer
  REQUIRE(_getdllprocaddr(kernel(), nullptr, 0) == nullptr);
}

TEST_CASE("getdllprocaddr: an invalid module handle is null, not a fault")
{
  REQUIRE(_getdllprocaddr(0, const_cast<char *>("GetTickCount"), -1) ==
          nullptr);
  REQUIRE(_getdllprocaddr((intptr_t)0x1234,
                          const_cast<char *>("GetTickCount"), -1) == nullptr);
  REQUIRE(_getdllprocaddr(0, nullptr, 1) == nullptr);
}

TEST_CASE("getdllprocaddr: the resolved pointer is callable")
{
  REQUIRE(kernel() != 0);

  proc_address_t fn =
      _getdllprocaddr(kernel(), const_cast<char *>("GetCurrentProcessId"), -1);
  REQUIRE(fn != nullptr);
  REQUIRE(fn() > 0);
}

TEST_CASE("getdllprocaddr: a UTF-8 export name survives the transcode")
{
  // kernel32 has no non-ASCII export, so the observable outcome is the
  // same null -- what is under test is that a name the transcode cannot
  // represent is a null lookup and not a crash
  REQUIRE(kernel() != 0);
  REQUIRE(_getdllprocaddr(kernel(), const_cast<char *>("\xE5\x87\xBD\xE6\x95\xB0"),
                          -1) == nullptr);
}

TEST_CASE("getdllprocaddr: the resolved entry is stable across calls")
{
  REQUIRE(kernel() != 0);

  proc_address_t a =
      _getdllprocaddr(kernel(), const_cast<char *>("GetTickCount"), -1);
  proc_address_t b =
      _getdllprocaddr(kernel(), const_cast<char *>("GetTickCount"), -1);
  REQUIRE(a == b);
  REQUIRE(a != nullptr);
}
