function crtdll_utf8_files()
  return {
    'crtdll/u/environment/__initenv.cc',
    'crtdll/u/environment/__p__environ.cc',
    'crtdll/u/environment/_putenv.cc',
    'crtdll/u/environment/getenv.cc',
    'crtdll/u/environment/putenv.cc',
    'crtdll/u/filesystem/_chmod.cc',
    'crtdll/u/filesystem/_findfirst.cc',
    'crtdll/u/filesystem/_findfirst32.cc',
    'crtdll/u/filesystem/_findfirst32i64.cc',
    'crtdll/u/filesystem/_findfirst64.cc',
    'crtdll/u/filesystem/_findfirst64i32.cc',
    'crtdll/u/filesystem/_findfirsti64.cc',
    'crtdll/u/filesystem/_findnext.cc',
    'crtdll/u/filesystem/_findnext32.cc',
    'crtdll/u/filesystem/_findnext32i64.cc',
    'crtdll/u/filesystem/_findnext64.cc',
    'crtdll/u/filesystem/_findnext64i32.cc',
    'crtdll/u/filesystem/_findnexti64.cc',
    'crtdll/u/filesystem/_fullpath.cc',
    'crtdll/u/filesystem/_stat.cc',
    'crtdll/u/filesystem/_stat32.cc',
    'crtdll/u/filesystem/_stat32i64.cc',
    'crtdll/u/filesystem/_stat64.cc',
    'crtdll/u/filesystem/_stat64i32.cc',
    'crtdll/u/filesystem/_stati64.cc',
    'crtdll/u/filesystem/_unlink.cc',
    'crtdll/u/filesystem/remove.cc',
    'crtdll/u/filesystem/rename.cc',
    'crtdll/u/filesystem/unlink.cc',
    'crtdll/u/runtime/__p___argv.cc',
    'crtdll/u/stdio/_getcwd.cc',
    'crtdll/u/stdio/_open.cc',
    'crtdll/u/stdio/_read.cc',
    'crtdll/u/stdio/_write.cc',
    'crtdll/u/stdio/fflush.cc',
    'crtdll/u/stdio/fgetc.cc',
    'crtdll/u/stdio/fgets.cc',
    'crtdll/u/stdio/fopen.cc',
    'crtdll/u/stdio/fputc.cc',
    'crtdll/u/stdio/fputs.cc',
    'crtdll/u/stdio/fread.cc',
    'crtdll/u/stdio/fwrite.cc',
    'crtdll/u/stdio/getc.cc',
    'crtdll/u/stdio/getcwd.cc',
    'crtdll/u/stdio/open.cc',
    'crtdll/u/stdio/putc.cc',
    'crtdll/u/stdio/puts.cc',
    'crtdll/u/stdio/read.cc',
    'crtdll/u/stdio/ungetc.cc',
    'crtdll/u/stdio/write.cc',
  }
end

function crtdll_utf8_startup_deps()
  -- symbols that are referenced by startup object
  -- they should be explicitly added to test target to be chosen
  return {
    'crtdll/u/environment/__initenv.cc',
    'crtdll/u/environment/__p__environ.cc',
    'crtdll/u/runtime/__GetMainArgs.cc',
    'crtdll/u/runtime/__p___argv.cc',
    'crtdll/w/__wgetmainargs.cc',
  }
end

function until_mingw(version)
  return function(target_version)
    return target_version < version
  end
end

function crtdll_thunk_core_vc20()
  return {
    {'_beginthreadex'},
    {'_endthreadex'},
  }
end

function crtdll_thunk_core_5_0()
  return {
    {'_fstat32i64', v = until_mingw(12)},
    {'_fstat64', v = until_mingw(13)},
    {'_wstat32i64', v = until_mingw(12)},
    {'_wstat64', v = until_mingw(13)},
  }
end

function crtdll_thunk_core_5_1()
  return {
    {'_aligned_free', v = until_mingw(13)},
    {'_aligned_malloc', v = until_mingw(13)},
  }
end

function crtdll_thunk_toolchain_vc20()
  return {
    {'_beginthreadex'},
    {'_endthreadex'},
  }
end

function crtdll_thunk_toolchain_5_0()
  return {
    {'_ctime64', v = until_mingw(14)},
    {'_fstat32i64', v = until_mingw(12)},
    {'_fstat64', v = until_mingw(13)},
    {'_futime64', v = until_mingw(14)},
    {'_gmtime64', v = until_mingw(14)},
    {'_localtime64', v = until_mingw(14)},
    {'_time64', v = until_mingw(13)},
    {'_wfindfirst64', v = until_mingw(14)},
    {'_wfindnext64', v = until_mingw(14)},
    {'_wstat32i64', v = until_mingw(12)},
    {'_wstat64', v = until_mingw(13)},
    {'_wutime64', v = until_mingw(14)},
  }
end

function crtdll_thunk_toolchain_5_1()
  return {
    {'_aligned_free', v = until_mingw(13)},
    {'_aligned_malloc', v = until_mingw(13)},
  }
end

function crtdll_thunk_toolchain_6_0()
  return {
    {'_wputenv_s'},
    {'wcscat_s'},
    {'wcscpy_s'},
    {'wcsncat_s'},
    {'wcsncpy_s'},
    {'wcstok_s', v = until_mingw(13)},
  }
end

function crtdll_thunk_toolchain_a_5_0()
  return {
    {'_findfirst64', v = until_mingw(14)},
    {'_findnext64', v = until_mingw(14)},
    {'_stat32i64', v = until_mingw(12)},
    {'_stat64', v = until_mingw(13)},
    {'_utime64', v = until_mingw(14)},
  }
end

function add_crtdll_sources(functions, prefix)
  local target_version = tonumber(get_config('mingw-version'))
  for _, entry in ipairs(functions) do
    local func_name = entry[1]
    local condition = entry.v

    if condition == nil or condition(target_version) then
      add_files(prefix .. '/' .. func_name .. '.cc')
    end
  end
end

target('alias-short-crtdll')
  enable_if_x86_32()
  on_build(build_short_import_library('def/lib32/crtdll.def'))
  set_kind('static')

target('overlay-crtdll')
  add_defines('__CRTDLL__')
  enable_if_x86_32()
  enable_thunk_options()

  if profile_core() then
    if ntddi_version() < ntddi_win4() then
      add_crtdll_sources(crtdll_thunk_core_4_0(), 'crtdll/4.0')
      add_files(
        'crtdll/w/__wgetmainargs.cc',
        'crtdll/w/_wchdir.cc',
        'crtdll/w/_wchmod.cc',
        'crtdll/w/_wfindfirst.cc',
        'crtdll/w/_wfindfirst32.cc',
        'crtdll/w/_wfindfirst32i64.cc',
        'crtdll/w/_wfindfirsti64.cc',
        'crtdll/w/_wfindnext.cc',
        'crtdll/w/_wfindnext32.cc',
        'crtdll/w/_wfindnext32i64.cc',
        'crtdll/w/_wfindnexti64.cc',
        'crtdll/w/_wfopen.cc',
        'crtdll/w/_wfullpath.cc',
        'crtdll/w/_wgetcwd.cc',
        'crtdll/w/_wmkdir.cc',
        'crtdll/w/_wopen.cc',
        'crtdll/w/_wstat32.cc',
        'crtdll/w/_wstat32i64.cc',
        'crtdll/w/_wstati64.cc',
        'crtdll/w/_wutime.cc',
        'crtdll/w/_wutime32.cc')
    end
    add_crtdll_sources(crtdll_thunk_core_vc20(), 'crtdll/vc20')
  end

  if profile_toolchain_or_utf8() then
    if ntddi_version() < ntddi_win4() then
      add_crtdll_sources(crtdll_thunk_toolchain_4_0(), 'crtdll/4.0')
      add_files(
        'crtdll/w/_wchdir.cc',
        'crtdll/w/_wchmod.cc',
        'crtdll/w/_wfindfirst.cc',
        'crtdll/w/_wfindfirst32.cc',
        'crtdll/w/_wfindfirst32i64.cc',
        'crtdll/w/_wfindfirsti64.cc',
        'crtdll/w/_wfindnext.cc',
        'crtdll/w/_wfindnext32.cc',
        'crtdll/w/_wfindnext32i64.cc',
        'crtdll/w/_wfindnexti64.cc',
        'crtdll/w/_wfopen.cc',
        'crtdll/w/_wfullpath.cc',
        'crtdll/w/_wgetcwd.cc',
        'crtdll/w/_wmkdir.cc',
        'crtdll/w/_wopen.cc',
        'crtdll/w/_wremove.cc',
        'crtdll/w/_wrename.cc',
        'crtdll/w/_wstat32.cc',
        'crtdll/w/_wstat32i64.cc',
        'crtdll/w/_wstati64.cc',
        'crtdll/w/_wunlink.cc',
        'crtdll/w/_wutime.cc',
        'crtdll/w/_wutime32.cc')
    end
    if ntddi_version() < ntddi_win2k() then
      add_crtdll_sources(crtdll_thunk_toolchain_5_0(), 'crtdll/5.0')
    end
    if ntddi_version() < ntddi_winxp() then
      add_crtdll_sources(crtdll_thunk_toolchain_5_1(), 'crtdll/5.1')
    end
    if ntddi_version() < ntddi_vista() then
      add_crtdll_sources(crtdll_thunk_toolchain_6_0(), 'crtdll/6.0')
    end
  end

  if profile_toolchain_utf8() then
    add_deps('utf8-musl.a')
    add_files(table.unpack(crtdll_utf8_files()))
    add_files(table.unpack(crtdll_utf8_startup_deps()))
    set_policy('build.merge_archive', true)
  end

  if profile_toolchain() then
    if ntddi_version() < ntddi_win2k() then
      add_crtdll_sources(crtdll_thunk_toolchain_a_5_0(), 'crtdll/a/5.0')
    end
  end

target('alias-long-crtdll')
  enable_if_x86_32()
  on_build(build_long_import_library('def/lib32/crtdll.def'))
  set_kind('static')
  skip_install()

target('thunk-crtdll')
  add_defines('__CRTDLL__')
  add_deps('alias-long-crtdll')
  add_files(
    'crtdll/6.0/_wputenv_s.cc',
    'crtdll/6.0/wcscat_s.cc',
    'crtdll/6.0/wcscpy_s.cc',
    'crtdll/6.0/wcsncat_s.cc',
    'crtdll/6.0/wcsncpy_s.cc',
    'crtdll/6.0/wcstok_s.cc')
  enable_if_x86_32()
  enable_thunk_options()
  merge_win32_alias()
  skip_install()

  if is_arch('i386', 'i686') then
    add_files(
      'crtdll/4.0/_beginthreadex.cc',
      'crtdll/5.0/_fstat64.cc',
      'crtdll/5.0/_futime64.cc',
      'crtdll/5.0/_gmtime64.cc',
      'crtdll/5.0/_localtime64.cc',
      'crtdll/5.0/_time64.cc',
      'crtdll/5.0/_wfindfirst64.cc',
      'crtdll/5.0/_wfindnext64.cc',
      'crtdll/5.0/_wstat64.cc',
      'crtdll/5.0/_wutime64.cc',
      'crtdll/5.0/fstat64.cc',
      'crtdll/5.1/_aligned_free.cc',
      'crtdll/5.1/_aligned_malloc.cc',
      'crtdll/w/__wgetmainargs.cc',
      'crtdll/w/_wchdir.cc',
      'crtdll/w/_wchmod.cc',
      'crtdll/w/_wfindfirst.cc',
      'crtdll/w/_wfindfirst32.cc',
      'crtdll/w/_wfindfirst32i64.cc',
      'crtdll/w/_wfindfirsti64.cc',
      'crtdll/w/_wfindnext.cc',
      'crtdll/w/_wfindnext32.cc',
      'crtdll/w/_wfindnext32i64.cc',
      'crtdll/w/_wfindnexti64.cc',
      'crtdll/w/_wfopen.cc',
      'crtdll/w/_wfullpath.cc',
      'crtdll/w/_wgetcwd.cc',
      'crtdll/w/_wmkdir.cc',
      'crtdll/w/_wopen.cc',
      'crtdll/w/_wstat32.cc',
      'crtdll/w/_wstat32i64.cc',
      'crtdll/w/_wstati64.cc',
      'crtdll/w/_wunlink.cc',
      'crtdll/w/_wutime.cc',
      'crtdll/w/_wutime32.cc')
  end

target('test-crtdll')
  add_tests('default')
  add_deps('thunk-crtdll')
  add_files(
    'crtdll/6.0/_wputenv_s.test.cc',
    'crtdll/6.0/wcscat_s.test.cc',
    'crtdll/6.0/wcscpy_s.test.cc',
    'crtdll/6.0/wcsncat_s.test.cc',
    'crtdll/6.0/wcsncpy_s.test.cc')
  add_linkorders('thunk-crtdll', 'crtdll')
  add_links('crtdll')
  enable_if_x86_32()
  enable_test_options()
  skip_install()

  if is_arch('i386', 'i686') then
    add_files(
      'crtdll/5.0/_ctime64.cc',
      'crtdll/5.0/_futime64.test.cc',
      'crtdll/5.0/_gmtime64.test.cc',
      'crtdll/5.0/_localtime64.test.cc',
      'crtdll/5.0/_wfindfirst64.test.cc',
      'crtdll/5.0/_wutime64.test.cc',
      'crtdll/5.1/_aligned_malloc.test.cc',
      'crtdll/w/_wfopen.test.cc')
  end

target('thunk-crtdll-a')
  add_defines('__CRTDLL__')
  add_deps('alias-long-crtdll')
  add_files(
    'crtdll/a/5.0/_findfirst64.cc',
    'crtdll/a/5.0/_findnext64.cc',
    'crtdll/a/5.0/_stat64.cc',
    'crtdll/a/5.0/_utime64.cc')
  enable_if_x86_32()
  enable_thunk_options()
  merge_win32_alias()
  skip_install()

target('test-crtdll-a')
  add_tests('default')
  add_deps('thunk-crtdll-a')
  add_files(
    'crtdll/a/5.0/_findfirst64.test.cc',
    'crtdll/a/5.0/_utime64.test.cc')
  add_linkorders('thunk-crtdll-a', 'crtdll')
  add_links('crtdll')
  enable_if_x86_32()
  enable_test_options()
  skip_install()

target('thunk-crtdll-u')
  add_defines('__CRTDLL__')
  add_deps('alias-long-crtdll', 'utf8-musl.a')
  add_files(table.unpack(crtdll_utf8_files()))
  enable_if_x86_32()
  enable_thunk_options()
  merge_win32_alias()
  skip_install()

target('test-crtdll-u')
  add_tests('default')
  add_deps('thunk-crtdll-u')
  add_files(
    'crtdll/u/stdio/_open.test.cc',
    'crtdll/u/stdio/fopen.test.cc')
  add_linkorders('thunk-crtdll-u', 'crtdll')
  add_links('crtdll')
  enable_if_x86_32()
  enable_test_options()
  skip_install()

target('console-crtdll')
  add_cxflags('-fno-builtin')
  add_defines('__CRTDLL__')
  add_deps('thunk-crtdll-u', 'utf8-musl.a')
  add_files('test/console.c')
  add_linkorders('thunk-crtdll-u', 'utf8-musl.a', 'crtdll')
  add_links('crtdll')
  enable_if_x86_32()
  enable_test_options()
  skip_install()

target('argv-crtdll')
  add_cxxflags('-nostdinc++')
  add_defines('__CRTDLL__')
  add_deps('thunk-crtdll-u')
  add_files('test/argv.c')
  add_files(table.unpack(crtdll_utf8_startup_deps()))
  add_linkorders('thunk-crtdll-u', 'crtdll')
  add_links('crtdll')
  add_tests('default', {
    runargs = {"你好", "世界"},
    pass_outputs = "argv[1] = 你好\nargv[2] = 世界\n",
    plain = true,
  })
  enable_if_x86_32()
  enable_test_options()
  skip_install()
