set_plat('mingw')
add_rules('mode.release')
set_languages('c11', 'cxx17')

includes('dep/catch2')

function enable_common_options()
  add_cxxflags(
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
end

function enable_test_options()
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
end

option('ucrt')
  add_csnippets('ucrt', [[
    #if defined(_UCRT)
    #else
      #error MSVCRT
    #endif
  ]])

target('thunk-crtu')
  add_files('crtu/**.cc|**.test.cc')
  enable_common_options()
  set_exceptions('no-cxx')
  set_kind('static')

target('thunk-vcrt')
  enable_common_options()
  set_exceptions('no-cxx')
  set_kind('static')

  if is_arch('i386') then
    add_files(
      'vcrt/4.0/*.cc|*.test.cc',
      'vcrt/5.0/*.cc|*.test.cc',
      'vcrt/5.1/*.cc|*.test.cc')
  end
  if is_arch('i386') or is_arch('x86_64') then
    add_files('vcrt/6.0/*.cc|*.test.cc')
  end

target('thunk-vcrtw')
  add_files('vcrtw/**.cc|**.test.cc')
  enable_common_options()
  set_enabled(is_arch('i386') and not has_config('ucrt'))
  set_exceptions('no-cxx')
  set_kind('static')

target('thunk-win32')
  enable_common_options()
  set_exceptions('no-cxx')
  set_kind('static')

  if is_arch('i386') then
    add_files(
      'win32/3.9999+4.10/**.cc|**.test.cc',
      'win32/4.0/**.cc|**.test.cc',
      'win32/5.0/**.cc|**.test.cc',
      'win32/5.1/**.cc|**.test.cc',
      'win32/5.2/**.cc|**.test.cc')
  end
  if is_arch('i386') or is_arch('x86_64') then
    add_files(
      'win32/6.0/**.cc|**.test.cc',
      'win32/6.1/**.cc|**.test.cc',
      'win32/6.2/**.cc|**.test.cc')
  end

target('thunk-win32u')
  add_files('win32u/**.cc|**.test.cc')
  enable_common_options()
  set_exceptions('no-cxx')
  set_kind('static')

target('test-crtu')
  add_files('crtu/**.cc')
  add_tests('default')
  enable_test_options()

target('test-vcrt')
  add_tests('default')
  enable_test_options()

  if is_arch('i386') then
    add_files(
      'vcrt/4.0/*.cc',
      'vcrt/5.0/*.cc',
      'vcrt/5.1/*.cc',
      'vcrtw/**.cc')
  end

  if is_arch('i386') or is_arch('x86_64') then
    add_files(
      'vcrt/6.0/*.cc')
  end

target('test-vcrtw')
  add_files('vcrtw/**.cc')
  add_tests('default')
  enable_test_options()
  set_enabled(is_arch('i386') and not has_config('ucrt'))

target('test-win32')
  add_links('ws2_32')
  add_tests('default')
  enable_test_options()

  if is_arch('i386') then
    add_files(
      'win32/3.9999+4.10/**.cc',
      'win32/4.0/**.cc',
      'win32/5.0/**.cc',
      'win32/5.1/**.cc',
      'win32/5.2/**.cc')
  end

  if is_arch('i386') or is_arch('x86_64') then
    add_files(
      'win32/6.0/**.cc',
      'win32/6.1/**.cc',
      'win32/6.2/**.cc')
  end

target('test-win32u')
  add_files('win32u/**.cc')
  add_tests('default')
  enable_test_options()

target('console')
  add_deps('thunk-crtu')
  add_files('console/test.cc')
  enable_common_options()
