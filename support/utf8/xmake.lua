set_plat('mingw')
add_rules('mode.release')
set_languages('c11', 'cxx17')

includes('dep/catch2')

target('header')
  add_headerfiles(
    'include/u8crt/(**.h)',
    {prefixdir = 'include'})
  set_kind('headeronly')

target('u8crt')
  add_cxxflags(
    '-fno-threadsafe-statics',
    '-nostdinc++')
  add_defines(
    'NOSTL_NOCRT',
    'NS_NOCRT=u8crt::libc',
    'NS_NOSTL=u8crt::stl',
    {public = true})
  add_files('src/**.cc|**.test.cc')
  add_includedirs(
    'include',
    'include/u8crt')
  set_exceptions('no-cxx')
  set_kind('static')

target('test/unit')
  add_deps(
    'catch2',
    'u8crt')
  add_files('src/**.test.cc')
  add_ldflags(
    '-nostdlib',
    '-static')
  add_links('kernel32')
  add_tests('default')

target('test/startup/c')
  add_files('test/startup/main.c')
