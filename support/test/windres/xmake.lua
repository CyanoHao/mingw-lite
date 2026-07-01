target("windres/hello")
  set_languages("c11")
  add_files(
    "hello.c",
    "utf8-manifest.rc")
  add_tests("default", {pass_outputs = "Hello, world!\n"})
