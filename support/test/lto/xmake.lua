target("lto/lto")
  add_cxflags("-flto")
  add_files("lto.c")
  add_includedirs("..")
  -- do not use `set_policy("build.optimization.lto", true)`
  -- it adds `-flto` which overrides `-flto=auto`
  add_ldflags("-flto=auto", {force = true})
  add_tests("default")
