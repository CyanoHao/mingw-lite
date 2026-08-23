target('alias-short-user32')
  set_enabled(has_config('short-alias'))
  set_kind('static')

  if is_arch('i386', 'i686') then
    on_build(build_short_import_library({'def/lib32/user32.def'}))
  else
    on_build(build_short_import_library({'def/user32.def'}))
  end

target('overlay-user32')
  enable_thunk_options()

  if profile_toolchain_utf8() then
    add_files('user32/u/CharNextExA.cc')
  end

target('alias-long-user32')
  set_kind('static')
  skip_install()

  if is_arch('i386', 'i686') then
    on_build(build_long_import_library({'def/lib32/user32.def'}))
  else
    on_build(build_long_import_library({'def/user32.def'}))
  end

target('thunk-user32-u')
  add_deps('alias-long-user32')
  add_files('user32/u/CharNextExA.cc')
  enable_thunk_options()
  merge_win32_alias()
  skip_install()
