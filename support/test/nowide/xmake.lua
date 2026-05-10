target("nowide/args")
  add_files("args.cc")
  add_links("nowide")
  add_tests("default", {
    runargs = {"你好", "世界"},
    pass_outputs = "argv[1] = 你好\nargv[2] = 世界\n",
    plain = true,
  })
  set_languages("c++17")

target("nowide/convert")
  add_files("convert.cc")
  add_links("nowide")
  add_tests("default", {pass_outputs = "ok\n"})
  set_languages("c++17")

target("nowide/fopen")
  add_files("fopen.cc")
  add_links("nowide")
  add_tests("default", {pass_outputs = "Hello, world!\n"})
  set_languages("c++17")

target("nowide/fstream")
  add_files("fstream.cc")
  add_links("nowide")
  add_tests("default", {pass_outputs = "Hello, world!\n"})
  set_languages("c++17")
