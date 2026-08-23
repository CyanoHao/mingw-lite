#pragma once

#include "corecrt_internal.h"

#include <errno.h>
#include <stddef.h>

#include <sal.h>

namespace mingw_thunk::ucrt
{

  struct mbstate_t
  {
    unsigned long _Wchar;
    unsigned short _Byte, _State;
  };

  errno_t __cdecl _wctomb_internal(_Out_opt_ int *_SizeConverted,
                                   _Out_writes_opt_z_(_SizeInBytes) char *_MbCh,
                                   _In_ size_t _SizeInBytes,
                                   _In_ wchar_t _WCh,
                                   _Inout_ __crt_cached_ptd_host &_Ptd);

  _Success_(return != -1) int __cdecl
  _mbtowc_internal(_Pre_notnull_ _Post_z_ wchar_t *_DstCh,
                   _In_reads_or_z_opt_(_SrcSizeInBytes) char const *_SrcCh,
                   _In_ size_t _SrcSizeInBytes,
                   _Inout_ __crt_cached_ptd_host &_Ptd);

  namespace __crt_mbstring
  {
    // size_t __cdecl __c16rtomb_utf8(char *s, char16_t c16, mbstate_t *ps,
    // __crt_cached_ptd_host& ptd);
    size_t __cdecl __c32rtomb_utf8(char *s,
                                   char32_t c32,
                                   mbstate_t *ps,
                                   __crt_cached_ptd_host &ptd);
    // size_t __cdecl
    // __mbrtoc16_utf8(char16_t *pc32, const char *s, size_t n, mbstate_t *ps,
    // __crt_cached_ptd_host& ptd);
    size_t __cdecl __mbrtoc32_utf8(char32_t *pc32,
                                   const char *s,
                                   size_t n,
                                   mbstate_t *ps,
                                   __crt_cached_ptd_host &ptd);

    size_t __cdecl __mbrtowc_utf8(wchar_t *pwc,
                                  const char *s,
                                  size_t n,
                                  mbstate_t *ps,
                                  __crt_cached_ptd_host &ptd);
    // size_t __cdecl
    // __mbsrtowcs_utf8(wchar_t *dst, const char **src, size_t len, mbstate_t
    // *ps, __crt_cached_ptd_host& ptd); size_t __cdecl
    // __wcsrtombs_utf8(char *dst, const wchar_t **src, size_t len, mbstate_t
    // *ps, __crt_cached_ptd_host& ptd);

    constexpr size_t INVALID = static_cast<size_t>(-1);
    constexpr size_t INCOMPLETE = static_cast<size_t>(-2);

    size_t return_illegal_sequence(mbstate_t *ps, __crt_cached_ptd_host &ptd);
    size_t reset_and_return(size_t retval, mbstate_t *ps);
  } // namespace __crt_mbstring

} // namespace mingw_thunk::ucrt
