target('overlay-bcrypt')
  enable_thunk_options()

  if profile_toolchain_or_utf8() then
    if ntddi_version() < ntddi_vista() then
      add_files('bcrypt/6.0/BCryptGenRandom.cc')
    end
  end

target('bcrypt')
  add_deps('overlay-bcrypt')
  enable_squash_options()
