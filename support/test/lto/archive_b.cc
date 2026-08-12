extern int archive_a(int);

int archive_b(int n) {
  return archive_a(n) * 2;
}
