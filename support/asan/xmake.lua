-- support/asan/xmake.lua
-- Build a static libasan.a for MinGW-w64 (GCC).
-- Source is vendored from GCC 15.3.0 libsanitizer with MinGW-native rewrites
-- (sanitizer_win_defs.h, asan_globals_win.cpp, asan_win.cpp, asan_malloc_win.cpp,
-- sanitizer_win.cpp) so the build needs no -fms-extensions and works with GNU ld.
set_plat('mingw')
add_rules('mode.debug', 'mode.release', 'mode.minsizeel')
set_defaultmode('release')
set_languages('c11', 'c++17')

-- File lists transcribed verbatim from the upstream Makefile.am files
-- (GCC 15.3.0): asan/Makefile.am:20-50, sanitizer_common/Makefile.am:24-87,
-- interception/Makefile.am:16-20, lsan/Makefile.am:20-23.  Platform-specific
-- files (asan_linux.cpp, sanitizer_mac.cpp, ...) are kept; each is #if-gated
-- internally and compiles to an empty object on Windows, matching upstream.
local asan_files = {
  'asan_activation', 'asan_allocator', 'asan_debugging', 'asan_descriptions',
  'asan_errors', 'asan_fake_stack', 'asan_flags', 'asan_globals',
  'asan_interceptors', 'asan_interceptors_memintrinsics', 'asan_linux',
  'asan_mac', 'asan_malloc_linux', 'asan_malloc_mac', 'asan_malloc_win',
  'asan_memory_profile', 'asan_new_delete', 'asan_poisoning', 'asan_posix',
  'asan_premap_shadow', 'asan_report', 'asan_rtl', 'asan_shadow_setup',
  'asan_stack', 'asan_stats', 'asan_suppressions', 'asan_thread', 'asan_win',
  'asan_win_dynamic_runtime_thunk',
  -- Added by mingw-lite: global-variable registration (.CRT$XCU).  Upstream
  -- keeps this out of libasan and in a per-module thunk library, but for the
  -- static runtime it must live in libasan.a so __asan_dso_reg_hook (force-
  -- linked from asan_win.cpp via WIN_FORCE_LINK) is defined here.
  'asan_globals_win',
}

local sanitizer_common_files = {
  'sancov_flags', 'sanitizer_allocator', 'sanitizer_allocator_checks',
  'sanitizer_allocator_report', 'sanitizer_common', 'sanitizer_common_libcdep',
  'sanitizer_coverage_libcdep_new', 'sanitizer_deadlock_detector1',
  'sanitizer_deadlock_detector2', 'sanitizer_dl', 'sanitizer_errno',
  'sanitizer_file', 'sanitizer_flags', 'sanitizer_flag_parser',
  'sanitizer_libc', 'sanitizer_libignore', 'sanitizer_linux',
  'sanitizer_linux_libcdep', 'sanitizer_linux_s390', 'sanitizer_mac',
  'sanitizer_mac_libcdep', 'sanitizer_mutex', 'sanitizer_netbsd',
  'sanitizer_platform_limits_freebsd', 'sanitizer_platform_limits_linux',
  'sanitizer_platform_limits_posix', 'sanitizer_platform_limits_solaris',
  'sanitizer_posix', 'sanitizer_posix_libcdep', 'sanitizer_printf',
  'sanitizer_procmaps_bsd', 'sanitizer_procmaps_common',
  'sanitizer_procmaps_linux', 'sanitizer_procmaps_mac',
  'sanitizer_procmaps_solaris', 'sanitizer_range', 'sanitizer_solaris',
  'sanitizer_stack_store', 'sanitizer_stackdepot', 'sanitizer_stacktrace',
  'sanitizer_stacktrace_libcdep', 'sanitizer_stacktrace_sparc',
  'sanitizer_symbolizer_mac', 'sanitizer_symbolizer_report',
  'sanitizer_stacktrace_printer', 'sanitizer_stoptheworld_linux_libcdep',
  'sanitizer_stoptheworld_mac', 'sanitizer_suppressions', 'sanitizer_symbolizer',
  'sanitizer_symbolizer_libbacktrace', 'sanitizer_symbolizer_libcdep',
  'sanitizer_symbolizer_markup', 'sanitizer_symbolizer_posix_libcdep',
  'sanitizer_symbolizer_win', 'sanitizer_termination',
  'sanitizer_thread_arg_retval', 'sanitizer_thread_history',
  'sanitizer_thread_registry', 'sanitizer_tls_get_addr',
  'sanitizer_unwind_linux_libcdep', 'sanitizer_unwind_win', 'sanitizer_win',
  'sanitizer_win_interception',
}

local interception_files = {
  'interception_linux', 'interception_mac', 'interception_win',
  'interception_type_test',
}

local lsan_files = { 'lsan_common', 'lsan_common_linux', 'lsan_common_mac' }

local function prefixed(list, dir)
  local r = {}
  for _, v in ipairs(list) do r[#r + 1] = dir .. '/' .. v .. '.cpp' end
  return r
end

target('asan')
  set_kind('static')
  set_basename('asan')  -- emit libasan.a
  -- AM_CXXFLAGS from the upstream Makefile.am (raw-cxx: no exceptions/rtti).
  add_cxxflags(
    '-Wall', '-W', '-Wno-unused-parameter', '-Wwrite-strings', '-Wno-long-long',
    '-fno-builtin', '-fno-exceptions', '-fno-rtti', '-fomit-frame-pointer',
    '-funwind-tables', '-fvisibility=hidden', '-Wno-variadic-macros'
  )
  -- DEFS from asan/Makefile.am:6 and sanitizer_common/Makefile.am:6.
  add_defines(
    '_GNU_SOURCE', '_DEBUG',
    '__STDC_CONSTANT_MACROS', '__STDC_FORMAT_MACROS', '__STDC_LIMIT_MACROS',
    'ASAN_HAS_EXCEPTIONS=1', 'ASAN_NEEDS_SEGV=1', 'CAN_SANITIZE_UB=0',
    'ASAN_HAS_CXA_RETHROW_PRIMARY_EXCEPTION=0'
  )
  -- AM_CPPFLAGS: -I include -I . (-isystem include/system).
  add_includedirs('src/include', 'src')
  add_sysincludedirs('src/include/system')
  add_files(prefixed(asan_files, 'src/asan'))
  add_files(prefixed(sanitizer_common_files, 'src/sanitizer_common'))
  add_files(prefixed(interception_files, 'src/interception'))
  add_files(prefixed(lsan_files, 'src/lsan'))
  add_files('src/asan/asan_interceptors_vfork.S')
  -- Public sanitizer headers (installed alongside the archive).
  add_headerfiles('src/include/(sanitizer/*.h)')
