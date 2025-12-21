# UTF-8 thunk
THUNK_LIST_TOOLCHAIN_UCRTU = {
  'api-ms-win-crt-stdio-l1-1-0': [
    '__stdio_common_vfprintf',
    '_close',
    '_open',
    'close',
    'fclose',
    'fopen',
    'fputc',
    'fputs',
    'open',
    'putc',
    'puts',
  ],
}

# feature thunk
THUNK_LIST_TOOLCHAIN_VCRT = {
  '4.0': [
    '_beginthreadex',
  ],
  '5.0': [
    '_ctime64',
    '_findfirst64',
    '_findnext64',
    ('_fstat64', lambda v_major: v_major < 13),
    '_futime64',
    ('_gmtime64', lambda v_major: v_major < 14),
    '_localtime64',
    ('_stat64', lambda v_major: v_major < 13),
    ('_time64', lambda v_major: v_major < 13),
    '_utime64',
    '_wfindfirst64',
    '_wfindnext64',
    ('_wstat64', lambda v_major: v_major < 13),
    '_wutime64',
  ],
  '5.1': [
    ('_aligned_free', lambda v_major: v_major < 13),
    ('_aligned_malloc', lambda v_major: v_major < 13),
  ],
  '6.0': [
    '_wputenv_s',
    'wcscat_s',
    'wcscpy_s',
    'wcsncat_s',
    'wcsncpy_s',
    # added in 13 to implement C95 wcstok
    ('wcstok_s', lambda v_major: v_major < 13),
  ],
}

# UTF-8 thunk
THUNK_LIST_TOOLCHAIN_VCRTU = [
  '_close',
  '_open',
  'fclose',
  'fopen',
  'fputc',
  'fputs',
  'open',
  'putc',
  'puts',
]

# wide thunk
THUNK_LIST_TOOLCHAIN_VCRTW = [
  '_wchdir',
  '_wchmod',
  '_wfindfirst',
  '_wfindfirst32',
  '_wfindfirst32i64',
  '_wfindfirsti64',
  '_wfindnext',
  '_wfindnext32',
  '_wfindnext32i64',
  '_wfindnexti64',
  '_wfopen',
  '_wfullpath',
  '_wgetcwd',
  '_wmkdir',
  '_wopen',
  '_wstat32',
  ('_wstat32i64', lambda v_major : v_major >= 12),
  '_wstati64',
  '_wutime',
  '_wutime32',
]

# feature thunk (-W only)
THUNK_LIST_TOOLCHAIN_WIN32 = {
  '3.9999+4.10': {
    'kernel32': [
      'CancelIo',
      'CreateWaitableTimerA',
      'GetFileAttributesExW',
      'GetFileAttributesExA',
      'IsDebuggerPresent',
      'MultiByteToWideChar',
      'SetProcessAffinityMask',
      'SetWaitableTimer',
      'SwitchToThread',
      'WideCharToMultiByte',
    ]
  },
  '4.0': {
    'kernel32': [
      'CopyFileExA',
      'CreateThread',
      'GetHandleInformation',
      'LockFileEx',
      'MoveFileExA',
      'SetHandleInformation',
      'TryEnterCriticalSection',
      'UnlockFileEx',
    ],
    'shell32': [
      'CommandLineToArgvW',
    ],
  },
  '5.0': {
    'advapi32': [
      'ConvertStringSecurityDescriptorToSecurityDescriptorW',
    ],
    'kernel32': [
      'CreateHardLinkW',
      'CreateToolhelp32Snapshot',
      'FindFirstVolumeW',
      'FindNextVolumeW',
      'FindVolumeClose',
      'GetCPInfoExA',
      'GetFileSizeEx',
      'GetLongPathNameW',
      'GetVolumePathNameW',
      'GlobalMemoryStatusEx',
      'Module32First',
      'Module32Next',
      'SetFilePointerEx',
    ],
  },
  '5.1': {
    'kernel32': [
      'AddVectoredExceptionHandler',
      'GetConsoleProcessList',
      'GetModuleHandleExW',
      'GetNumaHighestNodeNumber',
      'GetNumaNodeProcessorMask',
      'GetSystemTimes',
      'GetSystemWow64DirectoryA',
      'GetVolumePathNamesForVolumeNameW',
      'LCMapStringW',
      'RemoveVectoredExceptionHandler',
    ],
    'ws2_32': [
      'freeaddrinfo',
      'getaddrinfo',
      'getnameinfo',
    ],
  },
  '5.2': {
    'advapi32': [
      'RegDeleteKeyExA',
      'RegDeleteKeyExW',
    ],
    'kernel32': [
      'FlsAlloc',
      'FlsFree',
      'FlsGetValue',
      'FlsSetValue',
      'GetCurrentProcessorNumber',
      'GetLargePageMinimum',
      'NeedCurrentDirectoryForExePathA',
      'NeedCurrentDirectoryForExePathW',
      'SetThreadStackGuarantee',
    ],
  },
  '6.0': {
    'bcrypt': [
      'BCryptGenRandom',
    ],
    'kernel32': [
      'CancelIoEx',
      'CompareStringOrdinal',
      'CreateSymbolicLinkA',
      'CreateSymbolicLinkW',
      'CreateWaitableTimerExW',
      'DeleteProcThreadAttributeList',
      'GetFileInformationByHandleEx',
      'GetFinalPathNameByHandleW',
      'GetTickCount64',
      'InitializeProcThreadAttributeList',
      'LCMapStringEx',
      'LocaleNameToLCID',
      'SetFileInformationByHandle',
      'UpdateProcThreadAttribute',
    ],
  },
  '6.1': {
    'kernel32': [
      'GetActiveProcessorCount',
      'GetActiveProcessorGroupCount',
      'GetMaximumProcessorCount',
      'GetMaximumProcessorGroupCount',
      'SetWaitableTimerEx',
    ],
  },
  '6.2': {
    'api-ms-win-core-path-l1-1-0': [
      'PathAllocCanonicalize',
      'PathAllocCombine',
      'PathCchAddBackslash',
      'PathCchAddBackslashEx',
      'PathCchAddExtension',
      'PathCchAppend',
      'PathCchAppendEx',
      'PathCchCanonicalize',
      'PathCchCanonicalizeEx',
      'PathCchCombine',
      'PathCchCombineEx',
      'PathCchFindExtension',
      'PathCchIsRoot',
      'PathCchRemoveBackslash',
      'PathCchRemoveBackslashEx',
      'PathCchRemoveExtension',
      'PathCchRemoveFileSpec',
      'PathCchRenameExtension',
      'PathCchSkipRoot',
      'PathCchStripPrefix',
      'PathCchStripToRoot',
      'PathIsUNCEx',
    ],
    'kernel32': [
      'AddDllDirectory',
      'CopyFile2',
      'GetCurrentThreadStackLimits',
      'GetSystemTimePreciseAsFileTime',
      'RemoveDllDirectory',
    ],
  },
}

# pre-thunk for non-UTF-8 build (-A → -W)
THUNK_LIST_CORE_WIN32A = {
  '6.0': {
    'kernel32': [
      'GetFinalPathNameByHandleA',
    ],
  },
}

# UTF-8 thunk (-A → -W)
THUNK_LIST_TOOLCHAIN_WIN32U = {
  'kernel32': [
    'CreateFileA',
    'CreateProcessA',
    'GetACP',
    'GetFinalPathNameByHandleA',
    'GetFullPathNameA',
    'GetOEMCP',
    'WriteFile',
  ],
}

# wide thunk (-W → -A)
THUNK_LIST_TOOLCHAIN_WIN32W = {
  'kernel32': [
    'CopyFileExW',
    'CopyFileW',
    'CreateDirectoryW',
    'CreateEventW',
    'CreateFileW',
    'CreateProcessW',
    'CreateSemaphoreW',
    'CreateWaitableTimerW',
    'DeleteFileW',
    'FindFirstFileW',
    'FindNextFileW',
    'GetCurrentDirectoryW',
    'GetEnvironmentVariableW',
    'GetFileAttributesExW',
    'GetFileAttributesW',
    'GetFullPathNameW',
    'GetModuleFileNameW',
    'GetTempPathW',
    'MoveFileExW',
    'ReadDirectoryChangesW',
    'RemoveDirectoryW',
    'SetCurrentDirectoryW',
    'SetEnvironmentVariableW',
    'WriteConsoleW',
  ],
}
