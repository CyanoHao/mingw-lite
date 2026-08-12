int archive_a(int n) {
  int s = 0;
  for (int i = 0; i < n; ++i)
    s += i * i;
  return s;
}
