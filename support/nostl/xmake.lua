add_rules('mode.release')
set_languages('c99', 'c++14')
add_requires('catch2')

target('nostl')
    set_kind('headeronly')
    add_headerfiles('include/(nostl/**.*)')

target('test')
    set_kind('binary')
    add_packages('catch2')
    add_deps('nostl')
    add_files('test/**.cc')
    add_ldflags('-static')
