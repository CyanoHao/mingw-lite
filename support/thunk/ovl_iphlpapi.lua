target('overlay-iphlpapi')
  enable_thunk_options()

  if profile_qt() then
    if ntddi_version() < ntddi_vista() then
      add_files(
        'iphlpapi/6.0/ConvertInterfaceIndexToLuid.cc',
        'iphlpapi/6.0/ConvertInterfaceLuidToIndex.cc',
        'iphlpapi/6.0/ConvertInterfaceLuidToNameW.cc',
        'iphlpapi/6.0/ConvertInterfaceNameToLuidW.cc',
        'iphlpapi/6.0/GetAdaptersAddresses.cc')
    end
  end

target('thunk-iphlpapi')
  add_files(
    'iphlpapi/6.0/ConvertInterfaceIndexToLuid.cc',
    'iphlpapi/6.0/ConvertInterfaceLuidToIndex.cc',
    'iphlpapi/6.0/ConvertInterfaceLuidToNameW.cc',
    'iphlpapi/6.0/ConvertInterfaceNameToLuidW.cc',
    'iphlpapi/6.0/GetAdaptersAddresses.cc')
  enable_if_x86()
  enable_thunk_options()
  merge_win32_alias()
  skip_install()
