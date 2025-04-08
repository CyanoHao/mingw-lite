// Place in global namespace so GCC will check difinitions.

extern "C" unsigned int __sync_val_compare_and_swap_4(volatile void *ptr,
                                                      unsigned int oldval,
                                                      unsigned int newval)
{
  auto *ptr_ = reinterpret_cast<volatile unsigned int *>(ptr);

  while (true) {
    // Precheck, fast fail
    auto actual_oldval = *ptr_;
    if (oldval != actual_oldval) [[unlikely]] {
      return actual_oldval;
    }

    // Compare and swap
    bool ok = __atomic_compare_exchange_n(
        ptr_, &oldval, newval, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    if (ok) [[likely]] {
      return oldval;
    }

    // Occasionally, another thread beat us, try again.
    // It will likely fail in precheck.
  }
}
