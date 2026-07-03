# Files rewritten in this vendored tree

Base: GCC 15.3.0 `libsanitizer/` (LLVM merge `61a6439f35b6de28ff4aff4450d6fca970292fd5`
+ LOCAL_PATCHES `b53f7de3e6205f76a794e159a282193e2afaad16`, `4d2cd304714fddc8a995fc0311090fce7e70c122`).

Only these files diverge from the upstream GCC 15.3.0 source.  All changes replace
MSVC-only constructs (`__pragma(comment(linker,...))`, `#pragma section`,
`__declspec(allocate(...))`, `__declspec(thread)`, `#pragma comment(lib,...)`,
`#pragma comment(linker,"/merge:...")`) with GCC/MinGW-native equivalents so the
build needs **no `-fms-extensions`** and links with **GNU ld** (no lld-link).

| File | Change |
| --- | --- |
| `sanitizer_common/sanitizer_win_defs.h` | `WIN_WEAK_ALIAS` / `WIN_FORCE_LINK` / `WIN_EXPORT` / `WIN_WEAK_EXPORT_DEF` / `WIN_WEAK_IMPORT_DEF` redefined under `__MINGW32__ && __GNUC__` to use `__attribute__((weak, alias(...)))` and a `used` strong reference. MSVC path retained behind `#else`. |
| `asan/asan_globals_win.cpp` | `#pragma section` + `__declspec(allocate(".ASAN$GA/GZ"))` + `#pragma comment(linker,"/merge:.ASAN=.data")` → `__attribute__((section(...)))`; `.CRT$XCU`/`.CRT$XTX` likewise. |
| `asan/asan_win.cpp` | `__declspec(thread)` → `thread_local`; `.CRT$XCAB`/`.CRT$XLAB`/`.CRT$XLY` `#pragma section`+`__declspec(allocate)` → `__attribute__((section(...)))`. |
| `asan/asan_malloc_win.cpp` | `__declspec(noinline)` → `__attribute__((noinline))` (all malloc-family wrappers). `__declspec(dllexport)` left unchanged (MinGW GCC accepts it). |
| `sanitizer_common/sanitizer_win.cpp` | Removed `#pragma comment(lib,"psapi")` and `#pragma comment(lib,"synchronization.lib")` (those libs now come from `libsanitizer.spec`); `.CRT$XID` `#pragma section`+`__declspec(allocate)` → `__attribute__((section(...)))`. |

## File set compiled

Transcribed from the upstream Makefile.am files (GCC 15.3.0):
- `asan/Makefile.am:20-50` — `asan_files` (30), **plus** `asan_globals_win.cpp`
  (added by mingw-lite; see comment in `xmake.lua`).
- `sanitizer_common/Makefile.am:24-87` — `sanitizer_common_files` (47).
- `interception/Makefile.am:16-20` — `interception_files` (4).
- `lsan/Makefile.am:20-23` — `sanitizer_lsan_files` (3, linked into libasan).

**Not** compiled (intentionally): the three DLL-bridge thunks
(`asan_win_common_runtime_thunk.cpp`, `asan_win_static_runtime_thunk.cpp`,
`asan_malloc_win_thunk.cpp`) which use `__declspec(dllimport)` against the ASan
runtime — wrong for a statically-linked `libasan.a`.  `asan_globals_win.cpp` is
the only thunk file included, because static operation needs its `.CRT$XCU`
global-registration callback in the archive.

## Re-vendoring on upstream upgrade

1. Re-copy `asan/ sanitizer_common/ interception/ lsan/ include/` (source files
   only) from the new GCC libsanitizer tree.
2. Re-apply the divergences listed above (each is localised and commented with
   `MinGW-lite:`).
3. Refresh the file lists in `xmake.lua` from the new Makefile.am files.
