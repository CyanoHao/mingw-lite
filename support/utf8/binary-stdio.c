enum {
  _O_BINARY = 0x8000,
};

int _setmode(int fd, int mode);

#if 0
  Windows CRT's "text" mode I/O causes several problems:

  1. UCRT randomly eats linefeeds when piping on Windows Vista.
     Compiling any non-trivial program compiled with '-g3 -pipe'
     is likely to fail.

  2. MSVCRT, and UCRT prior to 10.0.14393, cannot write multi-byte
     characters one-char-by-one-char after set_locale(LC_ALL, "").

     MWE:

     int main()
     {
       setlocale(LC_ALL, "");
       for (auto ch : "你好")
         fputc(ch, stdout);
     }

     This is exactly how MinGW-w64's printf and toolchain's NLS work.
#endif

void __attribute__((constructor))
__mingw_lite_crt_set_binary_mode()
{
  _setmode(0, _O_BINARY);
  _setmode(1, _O_BINARY);
  _setmode(2, _O_BINARY);
}
