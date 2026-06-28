target('overlay-dwmapi')
  enable_thunk_options()

  if profile_qt() then
    if ntddi_version() < ntddi_vista() then
      add_files(
        'dwmapi/6.0/DwmEnableBlurBehindWindow.cc',
        'dwmapi/6.0/DwmGetWindowAttribute.cc',
        'dwmapi/6.0/DwmIsCompositionEnabled.cc',
        'dwmapi/6.0/DwmSetWindowAttribute.cc')
    end
  end

target('thunk-dwmapi')
  add_files(
    'dwmapi/6.0/DwmEnableBlurBehindWindow.cc',
    'dwmapi/6.0/DwmGetWindowAttribute.cc',
    'dwmapi/6.0/DwmIsCompositionEnabled.cc',
    'dwmapi/6.0/DwmSetWindowAttribute.cc')
  enable_if_x86()
  enable_thunk_options()
  merge_win32_alias()
  skip_install()
