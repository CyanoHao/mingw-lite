extern int __ms__access(const char *, int);

int access(const char *path, int mode) {
  return __ms__access(path, mode);
}

typeof(&access) dllimport_access asm("__imp__access") = &access;
