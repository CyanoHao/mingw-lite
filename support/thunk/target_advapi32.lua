target('overlay-advapi32')
  enable_thunk_options()

  if profile_toolchain_or_utf8() then
    if ntddi_version() < ntddi_winxp() then
      add_files(
        'advapi32/5.1/freeaddrinfo.cc',
        'advapi32/5.1/getaddrinfo.cc',
        'advapi32/5.1/getnameinfo.cc')
    end
  end

target('advapi32')
  add_deps('overlay-advapi32')
  enable_squash_options()

target('thunk-advapi32')
  add_files(
    'advapi32/5.1/freeaddrinfo.cc',
    'advapi32/5.1/getaddrinfo.cc',
    'advapi32/5.1/getnameinfo.cc')
  enable_thunk_options()
  set_enabled(is_arch('i386', 'i686'))
  skip_install()

target('test-advapi32')
  add_tests('default')
  add_deps('thunk-advapi32')
  add_files(
    'advapi32/5.1/getaddrinfo.test.cc',
    'advapi32/5.1/getnameinfo.test.cc')
  add_linkorders('thunk-advapi32', 'advapi32')
  add_links('advapi32')
  enable_test_options()
  set_enabled(is_arch('i386', 'i686'))
  skip_install()
