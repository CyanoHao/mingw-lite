THUNK_LIST_CORE = {
  '5.0': {},
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
