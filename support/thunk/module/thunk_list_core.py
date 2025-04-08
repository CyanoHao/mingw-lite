THUNK_LIST_CORE = {
  '4.0': {
    'kernel32': [
      '__sync_bool_compare_and_swap_4',
      '__sync_val_compare_and_swap_4',
    ],
  },
  '5.0': {
    'msvcrt': [
      ('_fstat64', lambda v_major: v_major < 13),
    ]
  },
  '5.1': {
    'kernel32': [
      # winpthreads: no longer required since 12
      ('AddVectoredExceptionHandler', lambda v_major: v_major < 12),
      ('RemoveVectoredExceptionHandler', lambda v_major: v_major < 12),
    ],
  },
  '5.2': {},
  '6.0': {},
}
