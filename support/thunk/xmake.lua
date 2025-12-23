set_plat('mingw')
add_rules('mode.release')
set_languages('c11', 'cxx17')

includes('dep/catch2')

target('thunk')
  add_cxxflags(
    '-fno-exceptions',
    '-fno-threadsafe-statics',
    '-nostdinc++')
  add_defines(
    'NOSTL_NOCRT',
    'NS_NOSTL=mingw_thunk::stl',
    'NS_NOCRT=mingw_thunk::libc',
    '_WIN32_WINNT=0x0f00',
    '__USE_MINGW_ANSI_STDIO=0')
  add_includedirs('include')
  set_exceptions('none')
  set_kind('static')

  if is_arch('i386') then
    add_files(
      'win32/3.9999+4.10/**.cc|**.test.cc',
      'win32/4.0/**.cc|**.test.cc',
      'win32/5.0/**.cc|**.test.cc',
      'win32/5.1/**.cc|**.test.cc',
      'win32/5.2/**.cc|**.test.cc',
      'win32w/**.cc|**.test.cc',
      'vcrt/4.0/*.cc|*.test.cc',
      'vcrt/5.0/*.cc|*.test.cc',
      'vcrt/5.1/*.cc|*.test.cc',
      'vcrtw/**.cc|**.test.cc')
  end
  if is_arch('i386') or is_arch('x86_64') then
    add_files(
      'win32/6.0/**.cc|**.test.cc',
      'win32/6.1/**.cc|**.test.cc',
      'win32/6.2/**.cc|**.test.cc',
      'vcrt/6.0/*.cc|*.test.cc')
  end

target('thunku')
  add_cxxflags(
    '-fno-threadsafe-statics',
    '-nostdinc++')
  add_defines(
    'NOSTL_NOCRT',
    'NS_NOCRT=mingw_thunk::libc',
    'NS_NOSTL=mingw_thunk::stl')
  add_files(
    'crtu/**.cc|**.test.cc',
    'win32u/**.cc|**.test.cc')
  add_includedirs('include')
  set_exceptions('no-cxx')
  set_kind('static')

target('test')
  add_defines(
    'ENABLE_TEST_OVERRIDE',
    'NOSTL_NOCRT',
    'NS_NOSTL=mingw_thunk::stl',
    'NS_NOCRT=mingw_thunk::libc',
    '_WIN32_WINNT=0x0f00',
    '__USE_MINGW_ANSI_STDIO=0')
  add_deps('catch2')
  add_includedirs('include')
  add_ldflags('-static')
  add_links('ws2_32')
  add_tests('default')
  set_exceptions('no-cxx')

  if is_arch('i386') then
    add_files(
      'win32/3.9999+4.10/**.cc',
      'win32/4.0/**.cc',
      'win32/5.0/**.cc',
      'win32/5.1/**.cc',
      'win32/5.2/**.cc',
      'win32w/**.cc',
      'vcrt/4.0/*.cc',
      'vcrt/5.0/*.cc',
      'vcrt/5.1/*.cc',
      'vcrtw/**.cc')
  end

  if is_arch('i386') or is_arch('x86_64') then
    add_files(
      'win32/6.0/**.cc',
      'win32/6.1/**.cc',
      'win32/6.2/**.cc',
      'vcrt/6.0/*.cc')
  end

target('testu')
  add_defines(
    'NOSTL_NOCRT',
    'NS_NOSTL=mingw_thunk::stl',
    'NS_NOCRT=mingw_thunk::libc',
    '_WIN32_WINNT=0x0f00',
    '__USE_MINGW_ANSI_STDIO=0')
  add_deps('catch2')
  add_files(
    'crtu/**.cc',
    'win32u/**.cc')
  add_includedirs('include')
  add_ldflags('-static')
  add_links('ws2_32')
  add_tests('default')
  set_exceptions('no-cxx')
