function u8crt_files()
  return {
    'u8crt/musl/internal/floatscan.cc',
    'u8crt/musl/internal/intscan.cc',
    'u8crt/musl/internal/shgetc.cc',
    'u8crt/musl/multibyte/internal.cc',
    'u8crt/musl/multibyte/mbrtoc16.cc',
    'u8crt/musl/multibyte/mbrtowc.cc',
    'u8crt/musl/multibyte/mbsinit.cc',
    'u8crt/musl/multibyte/wcrtomb.cc',
    'u8crt/musl/multibyte/wctomb.cc',
    'u8crt/musl/stdio/__console_close.cc',
    'u8crt/musl/stdio/__console_read.cc',
    'u8crt/musl/stdio/__console_seek.cc',
    'u8crt/musl/stdio/__console_write.cc',
    'u8crt/musl/stdio/__toread.cc',
    'u8crt/musl/stdio/__towrite.cc',
    'u8crt/musl/stdio/__uflow.cc',
    'u8crt/musl/stdio/fwrite.cc',
    'u8crt/musl/stdio/stderr.cc',
    'u8crt/musl/stdio/stdin.cc',
    'u8crt/musl/stdio/stdout.cc',
    'u8crt/musl/stdio/vfprintf.cc',
    'u8crt/musl/stdio/vfscanf.cc',
  }
end

target('ext-u8crt-shared')
  add_files(table.unpack(u8crt_files()))
  enable_thunk_options()
  set_basename('u8crt')
  set_kind('shared')

target('ext-u8crt-static')
  add_files(table.unpack(u8crt_files()))
  enable_thunk_options()
  set_basename('u8crt')
  set_kind('static')
