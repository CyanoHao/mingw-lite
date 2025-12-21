# feature thunk (-W only)
THUNK_LIST_CORE_WIN32 = {
  '3.9999+4.10': {
    'kernel32': [
      'GetFileAttributesExW',
      'GetFileAttributesExA',
      'IsDebuggerPresent',
      'SetProcessAffinityMask',
    ],
  },
  '4.0': {
    'kernel32': [
      'GetHandleInformation',
      'MoveFileExA',
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
  },
  '5.1': {
    'kernel32': [
      # winpthreads: no longer required since 12
      # but still required by python (see thunk_list_toolchain)
      ('AddVectoredExceptionHandler', lambda v_major: v_major < 12),
      ('RemoveVectoredExceptionHandler', lambda v_major: v_major < 12),
    ],
  },
  '5.2': {},
  '6.0': {},
}

# wide thunk (-W → -A)
THUNK_LIST_CORE_WIN32W = {
  'kernel32': [
    'CreateFileW',
    'DeleteFileW',
    'FindFirstFileW',
    'FindNextFileW',
    'GetCurrentDirectoryW',
    'GetFileAttributesExW',
    'GetFileAttributesW',
    'GetFullPathNameW',
    'MoveFileExW',
    'RemoveDirectoryW',
    'WriteConsoleW',
  ],
}
