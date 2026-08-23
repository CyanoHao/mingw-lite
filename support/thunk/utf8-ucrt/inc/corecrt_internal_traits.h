#pragma once

#include "corecrt_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <windows.h>

namespace mingw_thunk::ucrt
{

  template <typename Character>
  struct __crt_char_traits;

#define _CORECRT_APPLY_TO_MAPPINGS(_APPLY)                                     \
  _APPLY(capture_argv, __acrt_capture_narrow_argv, __acrt_capture_wide_argv)   \
  _APPLY(create_process, __acrt_CreateProcessA, CreateProcessW)                \
  _APPLY(find_first_file_ex, __acrt_FindFirstFileExA, FindFirstFileExW)        \
  _APPLY(find_next_file, __acrt_FindNextFileA, FindNextFileW)                  \
  _APPLY(free_environment_strings,                                             \
         FreeEnvironmentStringsA,                                              \
         FreeEnvironmentStringsW)                                              \
  _APPLY(ftprintf, fprintf, fwprintf)                                          \
  _APPLY(get_current_directory,                                                \
         __acrt_get_current_directory_narrow_acp_or_utf8,                      \
         __acrt_get_current_directory_wide)                                    \
  _APPLY(get_environment_from_os,                                              \
         __dcrt_get_narrow_environment_from_os,                                \
         __dcrt_get_wide_environment_from_os)                                  \
  _APPLY(get_full_path_name,                                                   \
         __acrt_get_full_path_name_narrow_acp_or_utf8,                         \
         __acrt_get_full_path_name_wide)                                       \
  _APPLY(get_module_file_name, __acrt_GetModuleFileNameA, GetModuleFileNameW)  \
  _APPLY(get_or_create_environment_nolock,                                     \
         __dcrt_get_or_create_narrow_environment_nolock,                       \
         __dcrt_get_or_create_wide_environment_nolock)                         \
  _APPLY(get_temp_path, __acrt_GetTempPath2A, __acrt_GetTempPath2W)            \
  _APPLY(getc_nolock, _getc_nolock, _getwc_nolock)                             \
  _APPLY(gettche_nolock, _getche_nolock, _getwche_nolock)                      \
  _APPLY(initialize_environment_nolock,                                        \
         _initialize_narrow_environment,                                       \
         _initialize_wide_environment)                                         \
  _APPLY(istspace, isspace, iswspace)                                          \
  _APPLY(itot_s, _itoa_s, _itow_s)                                             \
  _APPLY(message_box, __acrt_MessageBoxA, __acrt_MessageBoxW)                  \
  _APPLY(open_file, _openfile, _wopenfile)                                     \
  _APPLY(output_debug_string, __acrt_OutputDebugStringA, OutputDebugStringW)   \
  _APPLY(pack_command_line_and_environment,                                    \
         __acrt_pack_narrow_command_line_and_environment,                      \
         __acrt_pack_wide_command_line_and_environment)                        \
  _APPLY(                                                                      \
      puttc_nolock_internal, _fputc_nolock_internal, _fputwc_nolock_internal)  \
  _APPLY(                                                                      \
      puttch_nolock_internal, _putch_nolock_internal, _putwch_nolock_internal) \
  _APPLY(set_current_directory,                                                \
         __acrt_SetCurrentDirectoryA,                                          \
         SetCurrentDirectoryW)                                                 \
  _APPLY(set_environment_variable,                                             \
         __acrt_SetEnvironmentVariableA,                                       \
         SetEnvironmentVariableW)                                              \
  _APPLY(set_program_name, _set_pgmptr, _set_wpgmptr)                          \
  _APPLY(set_variable_in_environment_nolock,                                   \
         __dcrt_set_variable_in_narrow_environment_nolock,                     \
         __dcrt_set_variable_in_wide_environment_nolock)                       \
  _APPLY(show_message_box,                                                     \
         __acrt_show_narrow_message_box,                                       \
         __acrt_show_wide_message_box)                                         \
  _APPLY(sntprintf_s, _snprintf_s, _snwprintf_s)                               \
  _APPLY(taccess_s, _access_s, _waccess_s)                                     \
  _APPLY(tasctime, asctime, _wasctime)                                         \
  _APPLY(tasctime_s, asctime_s, _wasctime_s)                                   \
  _APPLY(tcscat_s, strcat_s, wcscat_s)                                         \
  _APPLY(tcschr, strchr, wcschr)                                               \
  _APPLY(tcscmp, strcmp, wcscmp)                                               \
  _APPLY(tcscpy_s, strcpy_s, wcscpy_s)                                         \
  _APPLY(tcserror_s, strerror_s, _wcserror_s)                                  \
  _APPLY(tcsicmp, _stricmp, _wcsicmp)                                          \
  _APPLY(tcslen, strlen, wcslen)                                               \
  _APPLY(tcsnlen_s, strnlen_s, wcsnlen_s)                                      \
  _APPLY(tcsncat_s, strncat_s, wcsncat_s)                                      \
  _APPLY(tcsncmp, strncmp, wcsncmp)                                            \
  _APPLY(tcsncpy_s, strncpy_s, wcsncpy_s)                                      \
  _APPLY(tcsnicmp, _strnicmp, _wcsnicmp)                                       \
  _APPLY(tcsnicoll, _strnicoll, _wcsnicoll)                                    \
  _APPLY(tcsnlen, strnlen, wcsnlen)                                            \
  _APPLY(tcspbrk, strpbrk, wcspbrk)                                            \
  _APPLY(tcsrchr, strrchr, wcsrchr)                                            \
  _APPLY(tcstoull, strtoull, wcstoull)                                         \
  _APPLY(tdupenv_s_crt, _dupenv_s_crt, _wdupenv_s_crt)                         \
  _APPLY(texecve, _execve, _wexecve)                                           \
  _APPLY(tfdopen, _fdopen, _wfdopen)                                           \
  _APPLY(tfullpath, _fullpath, _wfullpath)                                     \
  _APPLY(tgetcwd, _getcwd, _wgetcwd)                                           \
  _APPLY(tgetpath, __acrt_getpath, __acrt_wgetpath)                            \
  _APPLY(tmktemp_s, _mktemp_s, _wmktemp_s)                                     \
  _APPLY(tsopen_nolock, _sopen_nolock, _wsopen_nolock)                         \
  _APPLY(tsopen_s, _sopen_s, _wsopen_s)                                        \
  _APPLY(tspawnve, _spawnve, _wspawnve)                                        \
  _APPLY(tspawnvpe, _spawnvpe, _wspawnvpe)                                     \
  _APPLY(ulltot_s, _ui64toa_s, _ui64tow_s)                                     \
  _APPLY(ultot_s, _ultoa_s, _ultow_s)                                          \
  _APPLY(ungettc_nolock, _ungetc_nolock, _ungetwc_nolock)                      \
  _APPLY(ungettch_nolock, _ungetch_nolock, _ungetwch_nolock)

  template <>
  struct __crt_char_traits<char>
  {
    typedef char char_type;
    typedef unsigned char unsigned_char_type;
    typedef wchar_t other_char_type;
    typedef int int_type;
    typedef STARTUPINFOA startup_info_type;
    typedef WIN32_FIND_DATAA win32_find_data_type;

#define _APPLY(name, narrow_name, wide_name)                                   \
  _CORECRT_GENERATE_FORWARDER(static, __cdecl, name, narrow_name)
    _CORECRT_APPLY_TO_MAPPINGS(_APPLY)
#undef _APPLY
  };

  template <>
  struct __crt_char_traits<wchar_t>
  {
    typedef wchar_t char_type;
    typedef wchar_t unsigned_char_type;
    typedef char other_char_type;
    typedef wint_t int_type;
    typedef STARTUPINFOW startup_info_type;
    typedef WIN32_FIND_DATAW win32_find_data_type;

#define _APPLY(name, narrow_name, wide_name)                                   \
  _CORECRT_GENERATE_FORWARDER(static, __cdecl, name, wide_name)
    _CORECRT_APPLY_TO_MAPPINGS(_APPLY)
#undef _APPLY
  };

#undef _CORECRT_APPLY_TO_MAPPINGS

} // namespace mingw_thunk::ucrt
