// Place in global namespace so GCC will check difinitions.

extern "C" unsigned int __sync_fetch_and_add_4(volatile void *ptr,
                                               unsigned int value)
{
  return __atomic_fetch_add(
      reinterpret_cast<volatile unsigned int *>(ptr), value, __ATOMIC_SEQ_CST);
}
