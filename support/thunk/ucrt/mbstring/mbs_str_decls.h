#pragma once

// plan-3 §5.2 M13b: the 105 `_mbs*` string faces as the tests see them.
// One shared block rather than three copies of the same hundred lines.
//
// The block is unconditional, and that is the M12 finding again: i686
// hides most of these behind `_CRTIMP`, so nothing needs declaring, and
// x64 sees them and only warns about the dllimport attribute, which the
// plan accepts.  The `_s` family is not declared by <mbstring.h> at all
// -- the secure-crt multibyte surface lives in a header mingw does not
// ship -- so those forty-odd lines are load-bearing on both arches.

#include <errno.h>
#include <locale.h>
#include <stddef.h>

extern "C"
{
  errno_t __cdecl
  _mbscat_s(unsigned char *dst, size_t size, const unsigned char *src);
  errno_t __cdecl _mbscat_s_l(unsigned char *dst,
                              size_t size,
                              const unsigned char *src,
                              _locale_t locale);
  const unsigned char *__cdecl _mbschr(const unsigned char *string,
                                       unsigned int ch);
  const unsigned char *__cdecl
  _mbschr_l(const unsigned char *string, unsigned int ch, _locale_t locale);
  int __cdecl _mbscmp(const unsigned char *s1, const unsigned char *s2);
  int __cdecl
  _mbscmp_l(const unsigned char *s1, const unsigned char *s2, _locale_t locale);
  int __cdecl _mbscoll(const unsigned char *s1, const unsigned char *s2);
  int __cdecl _mbscoll_l(const unsigned char *s1,
                         const unsigned char *s2,
                         _locale_t locale);
  errno_t __cdecl
  _mbscpy_s(unsigned char *dst, size_t size, const unsigned char *src);
  errno_t __cdecl _mbscpy_s_l(unsigned char *dst,
                              size_t size,
                              const unsigned char *src,
                              _locale_t locale);
  size_t __cdecl _mbscspn(const unsigned char *string,
                          const unsigned char *control);
  size_t __cdecl _mbscspn_l(const unsigned char *string,
                            const unsigned char *control,
                            _locale_t locale);
  const unsigned char *__cdecl _mbsdec(const unsigned char *string,
                                       const unsigned char *current);
  const unsigned char *__cdecl _mbsdec_l(const unsigned char *string,
                                         const unsigned char *current,
                                         _locale_t locale);
  unsigned char *__cdecl _mbsdup(const unsigned char *string);
  int __cdecl _mbsicmp(const unsigned char *s1, const unsigned char *s2);
  int __cdecl _mbsicmp_l(const unsigned char *s1,
                         const unsigned char *s2,
                         _locale_t locale);
  int __cdecl _mbsicoll(const unsigned char *s1, const unsigned char *s2);
  int __cdecl _mbsicoll_l(const unsigned char *s1,
                          const unsigned char *s2,
                          _locale_t locale);
  const unsigned char *__cdecl _mbsinc(const unsigned char *current);
  const unsigned char *__cdecl _mbsinc_l(const unsigned char *current,
                                         _locale_t locale);
  size_t __cdecl _mbslen(const unsigned char *string);
  size_t __cdecl _mbslen_l(const unsigned char *string, _locale_t locale);
  unsigned char *__cdecl _mbslwr(unsigned char *string);
  unsigned char *__cdecl _mbslwr_l(unsigned char *string, _locale_t locale);
  errno_t __cdecl _mbslwr_s(unsigned char *string, size_t size);
  errno_t __cdecl
  _mbslwr_s_l(unsigned char *string, size_t size, _locale_t locale);
  unsigned char *__cdecl
  _mbsnbcat(unsigned char *dst, const unsigned char *src, size_t count);
  unsigned char *__cdecl _mbsnbcat_l(unsigned char *dst,
                                     const unsigned char *src,
                                     size_t count,
                                     _locale_t locale);
  errno_t __cdecl _mbsnbcat_s(unsigned char *dst,
                              size_t size,
                              const unsigned char *src,
                              size_t count);
  errno_t __cdecl _mbsnbcat_s_l(unsigned char *dst,
                                size_t size,
                                const unsigned char *src,
                                size_t count,
                                _locale_t locale);
  int __cdecl
  _mbsnbcmp(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnbcmp_l(const unsigned char *s1,
                          const unsigned char *s2,
                          size_t count,
                          _locale_t locale);
  size_t __cdecl _mbsnbcnt(const unsigned char *string, size_t char_count);
  size_t __cdecl
  _mbsnbcnt_l(const unsigned char *string, size_t char_count, _locale_t locale);
  int __cdecl
  _mbsnbcoll(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnbcoll_l(const unsigned char *s1,
                           const unsigned char *s2,
                           size_t count,
                           _locale_t locale);
  unsigned char *__cdecl
  _mbsnbcpy(unsigned char *dst, const unsigned char *src, size_t count);
  unsigned char *__cdecl _mbsnbcpy_l(unsigned char *dst,
                                     const unsigned char *src,
                                     size_t count,
                                     _locale_t locale);
  errno_t __cdecl _mbsnbcpy_s(unsigned char *dst,
                              size_t size,
                              const unsigned char *src,
                              size_t count);
  errno_t __cdecl _mbsnbcpy_s_l(unsigned char *dst,
                                size_t size,
                                const unsigned char *src,
                                size_t count,
                                _locale_t locale);
  int __cdecl
  _mbsnbicmp(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnbicmp_l(const unsigned char *s1,
                           const unsigned char *s2,
                           size_t count,
                           _locale_t locale);
  int __cdecl
  _mbsnbicoll(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnbicoll_l(const unsigned char *s1,
                            const unsigned char *s2,
                            size_t count,
                            _locale_t locale);
  unsigned char *__cdecl
  _mbsnbset(unsigned char *dst, unsigned int val, size_t count);
  unsigned char *__cdecl _mbsnbset_l(unsigned char *dst,
                                     unsigned int val,
                                     size_t count,
                                     _locale_t locale);
  errno_t __cdecl
  _mbsnbset_s(unsigned char *dst, size_t size, unsigned int val, size_t count);
  errno_t __cdecl _mbsnbset_s_l(unsigned char *dst,
                                size_t size,
                                unsigned int val,
                                size_t count,
                                _locale_t locale);
  unsigned char *__cdecl
  _mbsncat(unsigned char *dst, const unsigned char *src, size_t count);
  unsigned char *__cdecl _mbsncat_l(unsigned char *dst,
                                    const unsigned char *src,
                                    size_t count,
                                    _locale_t locale);
  errno_t __cdecl _mbsncat_s(unsigned char *dst,
                             size_t size,
                             const unsigned char *src,
                             size_t count);
  errno_t __cdecl _mbsncat_s_l(unsigned char *dst,
                               size_t size,
                               const unsigned char *src,
                               size_t count,
                               _locale_t locale);
  size_t __cdecl _mbsnccnt(const unsigned char *string, size_t byte_count);
  size_t __cdecl
  _mbsnccnt_l(const unsigned char *string, size_t byte_count, _locale_t locale);
  int __cdecl
  _mbsncmp(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsncmp_l(const unsigned char *s1,
                         const unsigned char *s2,
                         size_t count,
                         _locale_t locale);
  int __cdecl
  _mbsncoll(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsncoll_l(const unsigned char *s1,
                          const unsigned char *s2,
                          size_t count,
                          _locale_t locale);
  unsigned char *__cdecl
  _mbsncpy(unsigned char *dst, const unsigned char *src, size_t count);
  unsigned char *__cdecl _mbsncpy_l(unsigned char *dst,
                                    const unsigned char *src,
                                    size_t count,
                                    _locale_t locale);
  errno_t __cdecl _mbsncpy_s(unsigned char *dst,
                             size_t size,
                             const unsigned char *src,
                             size_t count);
  errno_t __cdecl _mbsncpy_s_l(unsigned char *dst,
                               size_t size,
                               const unsigned char *src,
                               size_t count,
                               _locale_t locale);
  unsigned int __cdecl _mbsnextc(const unsigned char *current);
  unsigned int __cdecl _mbsnextc_l(const unsigned char *current,
                                   _locale_t locale);
  int __cdecl
  _mbsnicmp(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnicmp_l(const unsigned char *s1,
                          const unsigned char *s2,
                          size_t count,
                          _locale_t locale);
  int __cdecl
  _mbsnicoll(const unsigned char *s1, const unsigned char *s2, size_t count);
  int __cdecl _mbsnicoll_l(const unsigned char *s1,
                           const unsigned char *s2,
                           size_t count,
                           _locale_t locale);
  const unsigned char *__cdecl _mbsninc(const unsigned char *string,
                                        size_t count);
  const unsigned char *__cdecl
  _mbsninc_l(const unsigned char *string, size_t count, _locale_t locale);
  size_t __cdecl _mbsnlen(const unsigned char *string, size_t count);
  size_t __cdecl
  _mbsnlen_l(const unsigned char *string, size_t count, _locale_t locale);
  unsigned char *__cdecl
  _mbsnset(unsigned char *dst, unsigned int val, size_t count);
  unsigned char *__cdecl _mbsnset_l(unsigned char *dst,
                                    unsigned int val,
                                    size_t count,
                                    _locale_t locale);
  errno_t __cdecl
  _mbsnset_s(unsigned char *dst, size_t size, unsigned int val, size_t count);
  errno_t __cdecl _mbsnset_s_l(unsigned char *dst,
                               size_t size,
                               unsigned int val,
                               size_t count,
                               _locale_t locale);
  const unsigned char *__cdecl _mbspbrk(const unsigned char *string,
                                        const unsigned char *control);
  const unsigned char *__cdecl _mbspbrk_l(const unsigned char *string,
                                          const unsigned char *control,
                                          _locale_t locale);
  const unsigned char *__cdecl _mbsrchr(const unsigned char *string,
                                        unsigned int ch);
  const unsigned char *__cdecl
  _mbsrchr_l(const unsigned char *string, unsigned int ch, _locale_t locale);
  unsigned char *__cdecl _mbsrev(unsigned char *string);
  unsigned char *__cdecl _mbsrev_l(unsigned char *string, _locale_t locale);
  unsigned char *__cdecl _mbsset(unsigned char *dst, unsigned int val);
  unsigned char *__cdecl
  _mbsset_l(unsigned char *dst, unsigned int val, _locale_t locale);
  errno_t __cdecl _mbsset_s(unsigned char *dst, size_t size, unsigned int val);
  errno_t __cdecl _mbsset_s_l(unsigned char *dst,
                              size_t size,
                              unsigned int val,
                              _locale_t locale);
  size_t __cdecl _mbsspn(const unsigned char *string,
                         const unsigned char *control);
  size_t __cdecl _mbsspn_l(const unsigned char *string,
                           const unsigned char *control,
                           _locale_t locale);
  const unsigned char *__cdecl _mbsspnp(const unsigned char *string,
                                        const unsigned char *control);
  const unsigned char *__cdecl _mbsspnp_l(const unsigned char *string,
                                          const unsigned char *control,
                                          _locale_t locale);
  const unsigned char *__cdecl _mbsstr(const unsigned char *string,
                                       const unsigned char *sub);
  const unsigned char *__cdecl _mbsstr_l(const unsigned char *string,
                                         const unsigned char *sub,
                                         _locale_t locale);
  unsigned char *__cdecl _mbstok(unsigned char *string,
                                 const unsigned char *control);
  unsigned char *__cdecl _mbstok_l(unsigned char *string,
                                   const unsigned char *control,
                                   _locale_t locale);
  unsigned char *__cdecl _mbstok_s(unsigned char *string,
                                   const unsigned char *control,
                                   unsigned char **context);
  unsigned char *__cdecl _mbstok_s_l(unsigned char *string,
                                     const unsigned char *control,
                                     unsigned char **context,
                                     _locale_t locale);
  size_t __cdecl _mbstrlen(const char *string);
  size_t __cdecl _mbstrlen_l(const char *string, _locale_t locale);
  size_t __cdecl _mbstrnlen(const char *string, size_t max_count);
  size_t __cdecl
  _mbstrnlen_l(const char *string, size_t max_count, _locale_t locale);
  unsigned char *__cdecl _mbsupr(unsigned char *string);
  unsigned char *__cdecl _mbsupr_l(unsigned char *string, _locale_t locale);
  errno_t __cdecl _mbsupr_s(unsigned char *string, size_t size);
  errno_t __cdecl
  _mbsupr_s_l(unsigned char *string, size_t size, _locale_t locale);
} // extern "C"
