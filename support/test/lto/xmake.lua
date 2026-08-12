target("lto/flags")
  -- do not use `set_policy("build.optimization.lto", true)`
  -- it adds `-flto` which overrides `-flto=auto`
  add_cxflags("-flto", {public = true})
  add_ldflags("-flto=auto", {force = true, public = true})
  add_shflags("-flto=auto", {force = true, public = true})
  set_kind("phony")

target("lto/bigobj-flags")
  add_cxflags("-Wa,-mbig-obj", {force = true, public = true})
  add_deps("lto/flags")
  set_kind("phony")

target("lto/lto")
  add_deps("lto/flags")
  add_files("lto.c")
  add_tests("default")

target("lto/cross-tu")
  add_deps("lto/flags")
  add_files("cross_tu_c.c", "cross_tu_cc.cc", "cross_tu_main.cc")
  add_tests("default", {pass_outputs = "sum = 171700\n"})
  set_languages("c11", "c++17")

target("lto/exceptions")
  add_deps("lto/flags")
  add_files("thrower.cc", "thrower_main.cc")
  add_tests("default", {
    pass_outputs =
      "invalid: negative code\n" ..
      "runtime: code 42\n" ..
      "caught = 2\n"})
  set_languages("c++17")

target("lto/dll-lib")
  add_deps("lto/flags")
  add_files("dll_lib.cc")
  set_basename("lto_dll_lib")
  set_enabled(has_config("dlopen"))
  set_kind("shared")
  set_languages("c++17")

target("lto/dll-main")
  add_deps("lto/dll-lib", "lto/flags")
  add_files("dll_main.cc")
  add_tests("default", {
    pass_outputs =
      "id = lto%-dll%-v1\n" ..
      "sum = 5050\n" ..
      "caught: from%-dll\n"})
  set_basename("lto_dll_main")
  set_enabled(has_config("dlopen"))
  set_languages("c++17")

target("lto/archive-lib")
  add_deps("lto/flags")
  add_files("archive_a.cc", "archive_b.cc")
  set_basename("lto_archive_lib")
  set_kind("static")
  set_languages("c++17")

target("lto/archive-main")
  add_deps("lto/archive-lib", "lto/flags")
  add_files("archive_main.cc")
  add_tests("default", {pass_outputs = "result = 570\n"})
  set_languages("c++17")

target("lto/tls")
  add_deps("lto/flags")
  add_files("tls_lib.cc", "tls_main.cc")
  add_tests("default", {
    pass_outputs =
      "local = 1000\n" ..
      "local = 1000\n" ..
      "local = 1000\n" ..
      "local = 1000\n" ..
      "total = 4000\n"})
  set_languages("c++17")

target("lto/bigobj")
  add_deps("lto/bigobj-flags")
  add_files("bigobj.c")
  add_tests("default")
  set_enabled(has_config("lto-bigobj"))

target("lto/bigobj-archive-lib")
  add_deps("lto/bigobj-flags")
  add_files("archive_a.cc", "archive_b.cc")
  set_basename("lto_bigobj_archive_lib")
  set_enabled(has_config("lto-bigobj"))
  set_kind("static")
  set_languages("c++17")

target("lto/bigobj-archive-main")
  add_deps("lto/bigobj-archive-lib", "lto/bigobj-flags")
  add_files("archive_main.cc")
  add_tests("default", {pass_outputs = "result = 570\n"})
  set_enabled(has_config("lto-bigobj"))
  set_languages("c++17")
