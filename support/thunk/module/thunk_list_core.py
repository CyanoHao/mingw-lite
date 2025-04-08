THUNK_LIST_CORE = {
  '3.9999+4.10': {
    'kernel32': [
      # followings are i386 atomic thunks
      # Windows 98 (4.10) claim to run on minimum i486, so we place them here
      'EnterCriticalSection',
      'LeaveCriticalSection',
      '__sync_bool_compare_and_swap_4',
      '__sync_val_compare_and_swap_4',
    ]
  },
  '4.0': {
    'kernel32': [
      'IsDebuggerPresent',
      'SetProcessAffinityMask',
      'TryEnterCriticalSection',
    ],
  },
  '5.0': {
    'kernel32': [
      'CreateHardLinkW',
      'FindFirstVolumeW',
      'FindNextVolumeW',
      'FindVolumeClose',
      'GetFileSizeEx',
    ],
    'msvcrt': [
      ('_fstat64', lambda v_major: v_major < 13),
      ('_wstat64', lambda v_major: v_major < 13),
    ]
  },
  '5.1': {
    'kernel32': [
      # winpthreads: no longer required since 12
      ('AddVectoredExceptionHandler', lambda v_major: v_major < 12),
      ('RemoveVectoredExceptionHandler', lambda v_major: v_major < 12),
    ],
    'msvcrt': [
      ('_aligned_free', lambda v_major: v_major < 13),
      ('_aligned_malloc', lambda v_major: v_major < 13),
    ],
  },
  '5.2': {},
  '6.0': {},
}
