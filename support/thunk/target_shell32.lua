target('overlay-shell32')
  enable_thunk_options()

  if profile_toolchain_or_utf8() then
    if ntddi_version() < ntddi_win4() then
      add_files('shell32/w/CommandLineToArgvW.cc')
    end
  end

target('shell32')
  add_deps('overlay-shell32')
  enable_squash_options()

target('thunk-shell32')
  add_files('shell32/w/CommandLineToArgvW.cc')
  enable_thunk_options()
  set_enabled(is_arch('i386', 'i686'))
  skip_install()

target('test-shell32')
  add_tests('default')
  add_deps('thunk-shell32')
  add_files('shell32/w/CommandLineToArgvW.test.cc')
  enable_test_options()
  set_enabled(is_arch('i386', 'i686'))
  skip_install()
