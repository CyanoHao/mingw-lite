// Place in global namespace so GCC will check difinitions.

extern "C" bool __sync_bool_compare_and_swap_4(volatile void *ptr,
                                               unsigned int expected,
                                               unsigned int desired)
{
  return __atomic_compare_exchange_n(
      reinterpret_cast<volatile unsigned int *>(ptr),
      &expected,
      desired,
      false,
      __ATOMIC_SEQ_CST,
      __ATOMIC_SEQ_CST);
}
