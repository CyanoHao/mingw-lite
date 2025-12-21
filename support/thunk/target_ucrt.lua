target('overlay-ucrt')
  add_defines('_UCRT')
  enable_thunk_options()

  if profile_toolchain_utf8() then
    add_files(
      'ucrt/filesystem/_chmod.cc',
      'ucrt/filesystem/_findfirst64.cc',
      'ucrt/filesystem/_findnext64.cc',
      'ucrt/filesystem/_fullpath.cc',
      'ucrt/filesystem/_stat64.cc',
      'ucrt/filesystem/_unlink.cc',
      'ucrt/filesystem/remove.cc',
      'ucrt/filesystem/rename.cc',
      'ucrt/stdio/__stdio_common_vfprintf.cc',
      'ucrt/stdio/_open.cc',
      'ucrt/stdio/fopen.cc',
      'ucrt/stdio/fputc.cc',
      'ucrt/stdio/fputs.cc',
      'ucrt/stdio/open.cc',
      'ucrt/stdio/putc.cc',
      'ucrt/stdio/puts.cc')
  end

target('ucrt')
  add_deps('overlay-ucrt')
  enable_squash_options()

target('thunk-ucrt-u')
  add_defines('_UCRT')
  add_files(
    'ucrt/filesystem/_chmod.cc',
    'ucrt/filesystem/_findfirst64.cc',
    'ucrt/filesystem/_findnext64.cc',
    'ucrt/filesystem/_fullpath.cc',
    'ucrt/filesystem/_stat64.cc',
    'ucrt/filesystem/_unlink.cc',
    'ucrt/filesystem/remove.cc',
    'ucrt/filesystem/rename.cc',
    'ucrt/stdio/__stdio_common_vfprintf.cc',
    'ucrt/stdio/_open.cc',
    'ucrt/stdio/fopen.cc',
    'ucrt/stdio/fputc.cc',
    'ucrt/stdio/fputs.cc',
    'ucrt/stdio/open.cc',
    'ucrt/stdio/putc.cc',
    'ucrt/stdio/puts.cc')
  enable_thunk_options()
  skip_install()

target('test-ucrt-u')
  add_tests('default')
  add_deps('thunk-ucrt-u')
  add_files(
    'ucrt/stdio/_open.test.cc',
    'ucrt/stdio/fopen.test.cc')
  add_linkorders('thunk-ucrt-u', 'ucrt')
  add_links('ucrt')
  enable_test_options()
  skip_install()

target('console-ucrt-u')
  add_defines('_UCRT')
  add_deps('thunk-ucrt-u')
  add_files('console/u.c')
  add_linkorders('thunk-ucrt-u', 'ucrt')
  add_links('ucrt')
  enable_test_options()
  skip_install()
