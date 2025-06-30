set_plat('mingw')
add_rules('mode.release')
set_languages('c99', 'cxx14')

target('intl')
  set_kind('static')
  add_cxxflags(
    '-fno-exceptions',
    '-fno-threadsafe-statics')
  add_includedirs('include', {public = true})
  set_exceptions('none')

  add_files('src/*.cc')

  add_headerfiles('include/libintl.h')

target('test')
  set_default(false)
  set_kind('binary')
  add_deps('intl')
  add_files('test/main.c')
