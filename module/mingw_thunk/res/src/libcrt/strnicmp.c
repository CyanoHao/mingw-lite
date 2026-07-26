extern int __ms__strnicmp(const char *, const char *, unsigned int);

int strnicmp(const char *a, const char *b, unsigned int n) {
  return __ms__strnicmp(a, b, n);
}

typeof(&strnicmp) dllimport_strnicmp asm("__imp__strnicmp") = &strnicmp;
