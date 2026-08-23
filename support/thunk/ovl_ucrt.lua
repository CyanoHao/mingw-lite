function ucrt_utf8_files()
  return {
    'ucrt/environment/__p__environ.cc',
    'ucrt/environment/_dupenv_s.cc',
    'ucrt/environment/_putenv.cc',
    'ucrt/environment/_putenv_s.cc',
    'ucrt/environment/_searchenv.cc',
    'ucrt/environment/_searchenv_s.cc',
    'ucrt/environment/_wputenv.cc',
    'ucrt/environment/_wputenv_s.cc',
    'ucrt/environment/getenv.cc',
    'ucrt/environment/getenv_s.cc',
    'ucrt/environment/putenv.cc',

    'ucrt/convert/c16rtomb.cc',
    'ucrt/convert/c32rtomb.cc',
    'ucrt/convert/mblen.cc',
    'ucrt/convert/mbrtoc16.cc',
    'ucrt/convert/mbrtoc32.cc',
    'ucrt/convert/mbrtowc.cc',
    'ucrt/convert/mbsrtowcs.cc',
    'ucrt/convert/mbsrtowcs_s.cc',
    'ucrt/convert/mbstowcs.cc',
    'ucrt/convert/mbstowcs_s.cc',
    'ucrt/convert/mbtowc.cc',
    'ucrt/convert/wcrtomb.cc',
    'ucrt/convert/wcsrtombs.cc',
    'ucrt/convert/wcsrtombs_s.cc',
    'ucrt/convert/wcstombs.cc',
    'ucrt/convert/wcstombs_s.cc',
    'ucrt/convert/wctomb.cc',
    'ucrt/convert/wctomb_s.cc',
    'ucrt/convert/_wcstombs_l.cc',
    'ucrt/convert/_wcstombs_s_l.cc',
    'ucrt/convert/_wctomb_l.cc',
    'ucrt/convert/_wctomb_s_l.cc',

    'ucrt/convert/atof.cc',
    'ucrt/convert/strtod.cc',
    'ucrt/convert/strtof.cc',
    'ucrt/convert/strtold.cc',
    'ucrt/convert/_atof_l.cc',
    'ucrt/convert/_strtod_l.cc',
    'ucrt/convert/_strtof_l.cc',
    'ucrt/convert/_strtold_l.cc',
    'ucrt/convert/wcstod.cc',
    'ucrt/convert/wcstof.cc',
    'ucrt/convert/wcstold.cc',
    'ucrt/convert/_wcstod_l.cc',
    'ucrt/convert/_wcstof_l.cc',
    'ucrt/convert/_wcstold_l.cc',
    'ucrt/convert/_wtof.cc',
    'ucrt/convert/_wtof_l.cc',
    'ucrt/convert/_atodbl.cc',
    'ucrt/convert/_atodbl_l.cc',
    'ucrt/convert/_atoflt.cc',
    'ucrt/convert/_atoflt_l.cc',
    'ucrt/convert/_atoldbl.cc',
    'ucrt/convert/_atoldbl_l.cc',
    'ucrt/convert/_ecvt.cc',
    'ucrt/convert/_ecvt_s.cc',
    'ucrt/convert/_fcvt.cc',
    'ucrt/convert/_fcvt_s.cc',
    'ucrt/convert/_gcvt.cc',
    'ucrt/convert/_gcvt_s.cc',

    'ucrt/conio/__conio_common_vcprintf.cc',
    'ucrt/conio/__conio_common_vcprintf_p.cc',
    'ucrt/conio/__conio_common_vcprintf_s.cc',
    'ucrt/conio/__conio_common_vcscanf.cc',
    'ucrt/conio/__conio_common_vcwprintf.cc',
    'ucrt/conio/__conio_common_vcwprintf_p.cc',
    'ucrt/conio/__conio_common_vcwprintf_s.cc',
    'ucrt/conio/__conio_common_vcwscanf.cc',
    'ucrt/conio/_cgets.cc',
    'ucrt/conio/_cgets_s.cc',
    'ucrt/conio/_cputs.cc',
    'ucrt/conio/_putch.cc',
    'ucrt/conio/_putch_nolock.cc',

    'ucrt/process/_execl.cc',
    'ucrt/process/_execle.cc',
    'ucrt/process/_execlp.cc',
    'ucrt/process/_execlpe.cc',
    'ucrt/process/_execv.cc',
    'ucrt/process/_execve.cc',
    'ucrt/process/_execvp.cc',
    'ucrt/process/_execvpe.cc',
    'ucrt/process/_loaddll.cc',
    'ucrt/process/_spawnl.cc',
    'ucrt/process/_spawnle.cc',
    'ucrt/process/_spawnlp.cc',
    'ucrt/process/_spawnlpe.cc',
    'ucrt/process/_spawnv.cc',
    'ucrt/process/_spawnve.cc',
    'ucrt/process/_spawnvp.cc',
    'ucrt/process/_spawnvpe.cc',

    'ucrt/filesystem/_access.cc',
    'ucrt/filesystem/_access_s.cc',
    'ucrt/filesystem/_chmod.cc',
    'ucrt/filesystem/_chdir.cc',
    'ucrt/filesystem/_findfirst.cc',
    'ucrt/filesystem/_findfirst32.cc',
    'ucrt/filesystem/_findfirst32i64.cc',
    'ucrt/filesystem/_findfirst64.cc',
    'ucrt/filesystem/_findfirst64i32.cc',
    'ucrt/filesystem/_findfirsti64.cc',
    'ucrt/filesystem/_findnext.cc',
    'ucrt/filesystem/_findnext32.cc',
    'ucrt/filesystem/_findnext32i64.cc',
    'ucrt/filesystem/_findnext64.cc',
    'ucrt/filesystem/_findnext64i32.cc',
    'ucrt/filesystem/_findnexti64.cc',
    'ucrt/filesystem/_fullpath.cc',
    'ucrt/filesystem/_makepath.cc',
    'ucrt/filesystem/_makepath_s.cc',
    'ucrt/filesystem/_mkdir.cc',
    'ucrt/filesystem/_rmdir.cc',
    'ucrt/filesystem/_splitpath.cc',
    'ucrt/filesystem/_splitpath_s.cc',
    'ucrt/filesystem/_stat.cc',
    'ucrt/filesystem/_stat32.cc',
    'ucrt/filesystem/_stat32i64.cc',
    'ucrt/filesystem/_stat64.cc',
    'ucrt/filesystem/_stat64i32.cc',
    'ucrt/filesystem/_stati64.cc',
    'ucrt/filesystem/_unlink.cc',
    'ucrt/filesystem/remove.cc',
    'ucrt/filesystem/rename.cc',
    'ucrt/filesystem/stat.cc',
    'ucrt/filesystem/stat32.cc',
    'ucrt/filesystem/stat32i64.cc',
    'ucrt/filesystem/stat64.cc',
    'ucrt/filesystem/stat64i32.cc',
    'ucrt/filesystem/unlink.cc',

    'ucrt/locale/___lc_codepage_func.cc',
    'ucrt/locale/___lc_collate_cp_func.cc',
    'ucrt/locale/___lc_locale_name_func.cc',
    'ucrt/locale/___mb_cur_max_func.cc',
    'ucrt/locale/___mb_cur_max_l_func.cc',
    'ucrt/locale/__pctype_func.cc',
    'ucrt/locale/__pwctype_func.cc',
    'ucrt/locale/_configthreadlocale.cc',
    'ucrt/locale/_create_locale.cc',
    'ucrt/locale/_free_locale.cc',
    'ucrt/locale/_get_current_locale.cc',
    'ucrt/locale/_getmbcp.cc',
    'ucrt/locale/_setmbcp.cc',
    'ucrt/locale/_wcreate_locale.cc',
    'ucrt/locale/_wsetlocale.cc',
    'ucrt/locale/setlocale.cc',
    'ucrt/locale/localeconv.cc',

    -- M13a (plan-3 5.2): the 70 classification / byte-role faces of
    -- the multibyte api-set.  See ucrt/mbstring/mbs_pred.h.
    'ucrt/mbstring/_ismbbalnum.cc',
    'ucrt/mbstring/_ismbbalnum_l.cc',
    'ucrt/mbstring/_ismbbalpha.cc',
    'ucrt/mbstring/_ismbbalpha_l.cc',
    'ucrt/mbstring/_ismbbblank.cc',
    'ucrt/mbstring/_ismbbblank_l.cc',
    'ucrt/mbstring/_ismbbgraph.cc',
    'ucrt/mbstring/_ismbbgraph_l.cc',
    'ucrt/mbstring/_ismbbkalnum.cc',
    'ucrt/mbstring/_ismbbkalnum_l.cc',
    'ucrt/mbstring/_ismbbkana.cc',
    'ucrt/mbstring/_ismbbkana_l.cc',
    'ucrt/mbstring/_ismbbkprint.cc',
    'ucrt/mbstring/_ismbbkprint_l.cc',
    'ucrt/mbstring/_ismbbkpunct.cc',
    'ucrt/mbstring/_ismbbkpunct_l.cc',
    'ucrt/mbstring/_ismbblead.cc',
    'ucrt/mbstring/_ismbblead_l.cc',
    'ucrt/mbstring/_ismbbprint.cc',
    'ucrt/mbstring/_ismbbprint_l.cc',
    'ucrt/mbstring/_ismbbpunct.cc',
    'ucrt/mbstring/_ismbbpunct_l.cc',
    'ucrt/mbstring/_ismbbtrail.cc',
    'ucrt/mbstring/_ismbbtrail_l.cc',
    'ucrt/mbstring/_ismbcalnum.cc',
    'ucrt/mbstring/_ismbcalnum_l.cc',
    'ucrt/mbstring/_ismbcalpha.cc',
    'ucrt/mbstring/_ismbcalpha_l.cc',
    'ucrt/mbstring/_ismbcblank.cc',
    'ucrt/mbstring/_ismbcblank_l.cc',
    'ucrt/mbstring/_ismbcdigit.cc',
    'ucrt/mbstring/_ismbcdigit_l.cc',
    'ucrt/mbstring/_ismbcgraph.cc',
    'ucrt/mbstring/_ismbcgraph_l.cc',
    'ucrt/mbstring/_ismbchira.cc',
    'ucrt/mbstring/_ismbchira_l.cc',
    'ucrt/mbstring/_ismbckata.cc',
    'ucrt/mbstring/_ismbckata_l.cc',
    'ucrt/mbstring/_ismbcl0.cc',
    'ucrt/mbstring/_ismbcl0_l.cc',
    'ucrt/mbstring/_ismbcl1.cc',
    'ucrt/mbstring/_ismbcl1_l.cc',
    'ucrt/mbstring/_ismbcl2.cc',
    'ucrt/mbstring/_ismbcl2_l.cc',
    'ucrt/mbstring/_ismbclegal.cc',
    'ucrt/mbstring/_ismbclegal_l.cc',
    'ucrt/mbstring/_ismbclower.cc',
    'ucrt/mbstring/_ismbclower_l.cc',
    'ucrt/mbstring/_ismbcprint.cc',
    'ucrt/mbstring/_ismbcprint_l.cc',
    'ucrt/mbstring/_ismbcpunct.cc',
    'ucrt/mbstring/_ismbcpunct_l.cc',
    'ucrt/mbstring/_ismbcspace.cc',
    'ucrt/mbstring/_ismbcspace_l.cc',
    'ucrt/mbstring/_ismbcsymbol.cc',
    'ucrt/mbstring/_ismbcsymbol_l.cc',
    'ucrt/mbstring/_ismbcupper.cc',
    'ucrt/mbstring/_ismbcupper_l.cc',
    'ucrt/mbstring/_ismbslead.cc',
    'ucrt/mbstring/_ismbslead_l.cc',
    'ucrt/mbstring/_ismbstrail.cc',
    'ucrt/mbstring/_ismbstrail_l.cc',
    'ucrt/mbstring/_mbbtype.cc',
    'ucrt/mbstring/_mbbtype_l.cc',
    'ucrt/mbstring/_mblen_l.cc',
    'ucrt/mbstring/_mbsbtype.cc',
    'ucrt/mbstring/_mbsbtype_l.cc',
    'ucrt/mbstring/_mbstowcs_l.cc',
    'ucrt/mbstring/_mbstowcs_s_l.cc',
    'ucrt/mbstring/_mbtowc_l.cc',

    -- M13b (plan-3 5.2): the 105 _mbs* string faces of the multibyte
    -- api-set -- walking, counting, comparing, locating, copy/cat/set,
    -- case, reverse and tokenize.  See ucrt/mbstring/mbs_str.h.
    'ucrt/mbstring/_mbscat_s.cc',
    'ucrt/mbstring/_mbscat_s_l.cc',
    'ucrt/mbstring/_mbschr.cc',
    'ucrt/mbstring/_mbschr_l.cc',
    'ucrt/mbstring/_mbscmp.cc',
    'ucrt/mbstring/_mbscmp_l.cc',
    'ucrt/mbstring/_mbscoll.cc',
    'ucrt/mbstring/_mbscoll_l.cc',
    'ucrt/mbstring/_mbscpy_s.cc',
    'ucrt/mbstring/_mbscpy_s_l.cc',
    'ucrt/mbstring/_mbscspn.cc',
    'ucrt/mbstring/_mbscspn_l.cc',
    'ucrt/mbstring/_mbsdec.cc',
    'ucrt/mbstring/_mbsdec_l.cc',
    'ucrt/mbstring/_mbsdup.cc',
    'ucrt/mbstring/_mbsicmp.cc',
    'ucrt/mbstring/_mbsicmp_l.cc',
    'ucrt/mbstring/_mbsicoll.cc',
    'ucrt/mbstring/_mbsicoll_l.cc',
    'ucrt/mbstring/_mbsinc.cc',
    'ucrt/mbstring/_mbsinc_l.cc',
    'ucrt/mbstring/_mbslen.cc',
    'ucrt/mbstring/_mbslen_l.cc',
    'ucrt/mbstring/_mbslwr.cc',
    'ucrt/mbstring/_mbslwr_l.cc',
    'ucrt/mbstring/_mbslwr_s.cc',
    'ucrt/mbstring/_mbslwr_s_l.cc',
    'ucrt/mbstring/_mbsnbcat.cc',
    'ucrt/mbstring/_mbsnbcat_l.cc',
    'ucrt/mbstring/_mbsnbcat_s.cc',
    'ucrt/mbstring/_mbsnbcat_s_l.cc',
    'ucrt/mbstring/_mbsnbcmp.cc',
    'ucrt/mbstring/_mbsnbcmp_l.cc',
    'ucrt/mbstring/_mbsnbcnt.cc',
    'ucrt/mbstring/_mbsnbcnt_l.cc',
    'ucrt/mbstring/_mbsnbcoll.cc',
    'ucrt/mbstring/_mbsnbcoll_l.cc',
    'ucrt/mbstring/_mbsnbcpy.cc',
    'ucrt/mbstring/_mbsnbcpy_l.cc',
    'ucrt/mbstring/_mbsnbcpy_s.cc',
    'ucrt/mbstring/_mbsnbcpy_s_l.cc',
    'ucrt/mbstring/_mbsnbicmp.cc',
    'ucrt/mbstring/_mbsnbicmp_l.cc',
    'ucrt/mbstring/_mbsnbicoll.cc',
    'ucrt/mbstring/_mbsnbicoll_l.cc',
    'ucrt/mbstring/_mbsnbset.cc',
    'ucrt/mbstring/_mbsnbset_l.cc',
    'ucrt/mbstring/_mbsnbset_s.cc',
    'ucrt/mbstring/_mbsnbset_s_l.cc',
    'ucrt/mbstring/_mbsncat.cc',
    'ucrt/mbstring/_mbsncat_l.cc',
    'ucrt/mbstring/_mbsncat_s.cc',
    'ucrt/mbstring/_mbsncat_s_l.cc',
    'ucrt/mbstring/_mbsnccnt.cc',
    'ucrt/mbstring/_mbsnccnt_l.cc',
    'ucrt/mbstring/_mbsncmp.cc',
    'ucrt/mbstring/_mbsncmp_l.cc',
    'ucrt/mbstring/_mbsncoll.cc',
    'ucrt/mbstring/_mbsncoll_l.cc',
    'ucrt/mbstring/_mbsncpy.cc',
    'ucrt/mbstring/_mbsncpy_l.cc',
    'ucrt/mbstring/_mbsncpy_s.cc',
    'ucrt/mbstring/_mbsncpy_s_l.cc',
    'ucrt/mbstring/_mbsnextc.cc',
    'ucrt/mbstring/_mbsnextc_l.cc',
    'ucrt/mbstring/_mbsnicmp.cc',
    'ucrt/mbstring/_mbsnicmp_l.cc',
    'ucrt/mbstring/_mbsnicoll.cc',
    'ucrt/mbstring/_mbsnicoll_l.cc',
    'ucrt/mbstring/_mbsninc.cc',
    'ucrt/mbstring/_mbsninc_l.cc',
    'ucrt/mbstring/_mbsnlen.cc',
    'ucrt/mbstring/_mbsnlen_l.cc',
    'ucrt/mbstring/_mbsnset.cc',
    'ucrt/mbstring/_mbsnset_l.cc',
    'ucrt/mbstring/_mbsnset_s.cc',
    'ucrt/mbstring/_mbsnset_s_l.cc',
    'ucrt/mbstring/_mbspbrk.cc',
    'ucrt/mbstring/_mbspbrk_l.cc',
    'ucrt/mbstring/_mbsrchr.cc',
    'ucrt/mbstring/_mbsrchr_l.cc',
    'ucrt/mbstring/_mbsrev.cc',
    'ucrt/mbstring/_mbsrev_l.cc',
    'ucrt/mbstring/_mbsset.cc',
    'ucrt/mbstring/_mbsset_l.cc',
    'ucrt/mbstring/_mbsset_s.cc',
    'ucrt/mbstring/_mbsset_s_l.cc',
    'ucrt/mbstring/_mbsspn.cc',
    'ucrt/mbstring/_mbsspn_l.cc',
    'ucrt/mbstring/_mbsspnp.cc',
    'ucrt/mbstring/_mbsspnp_l.cc',
    'ucrt/mbstring/_mbsstr.cc',
    'ucrt/mbstring/_mbsstr_l.cc',
    'ucrt/mbstring/_mbstok.cc',
    'ucrt/mbstring/_mbstok_l.cc',
    'ucrt/mbstring/_mbstok_s.cc',
    'ucrt/mbstring/_mbstok_s_l.cc',
    'ucrt/mbstring/_mbstrlen.cc',
    'ucrt/mbstring/_mbstrlen_l.cc',
    'ucrt/mbstring/_mbstrnlen.cc',
    'ucrt/mbstring/_mbstrnlen_l.cc',
    'ucrt/mbstring/_mbsupr.cc',
    'ucrt/mbstring/_mbsupr_l.cc',
    'ucrt/mbstring/_mbsupr_s.cc',
    'ucrt/mbstring/_mbsupr_s_l.cc',

    -- M13c: the single-character faces and the two pseudo-tables.  See
    -- ucrt/mbstring/mbs_char.h.  _mbcasemap is the api-set's one DATA
    -- export; __p__mbcasemap hands out the same bytes.
    'ucrt/mbstring/_mbbtombc.cc',
    'ucrt/mbstring/_mbbtombc_l.cc',
    'ucrt/mbstring/_mbcasemap.cc',
    'ucrt/mbstring/_mbccpy.cc',
    'ucrt/mbstring/_mbccpy_l.cc',
    'ucrt/mbstring/_mbccpy_s.cc',
    'ucrt/mbstring/_mbccpy_s_l.cc',
    'ucrt/mbstring/_mbcjistojms.cc',
    'ucrt/mbstring/_mbcjistojms_l.cc',
    'ucrt/mbstring/_mbcjmstojis.cc',
    'ucrt/mbstring/_mbcjmstojis_l.cc',
    'ucrt/mbstring/_mbclen.cc',
    'ucrt/mbstring/_mbclen_l.cc',
    'ucrt/mbstring/_mbctohira.cc',
    'ucrt/mbstring/_mbctohira_l.cc',
    'ucrt/mbstring/_mbctokata.cc',
    'ucrt/mbstring/_mbctokata_l.cc',
    'ucrt/mbstring/_mbctolower.cc',
    'ucrt/mbstring/_mbctolower_l.cc',
    'ucrt/mbstring/_mbctombb.cc',
    'ucrt/mbstring/_mbctombb_l.cc',
    'ucrt/mbstring/_mbctoupper.cc',
    'ucrt/mbstring/_mbctoupper_l.cc',
    'ucrt/mbstring/__p__mbcasemap.cc',
    'ucrt/mbstring/__p__mbctype.cc',

    'ucrt/string/__iscsym.cc',
    'ucrt/string/__iscsymf.cc',
    'ucrt/string/_isalnum_l.cc',
    'ucrt/string/_isalpha_l.cc',
    'ucrt/string/_isblank_l.cc',
    'ucrt/string/_iscntrl_l.cc',
    'ucrt/string/_isctype.cc',
    'ucrt/string/_isctype_l.cc',
    'ucrt/string/_isdigit_l.cc',
    'ucrt/string/_isgraph_l.cc',
    'ucrt/string/_isleadbyte_l.cc',
    'ucrt/string/_islower_l.cc',
    'ucrt/string/_isprint_l.cc',
    'ucrt/string/_ispunct_l.cc',
    'ucrt/string/_isspace_l.cc',
    'ucrt/string/_isupper_l.cc',
    'ucrt/string/_isxdigit_l.cc',
    'ucrt/string/_memicmp.cc',
    'ucrt/string/_memicmp_l.cc',
    'ucrt/string/_stricmp.cc',
    'ucrt/string/_stricmp_l.cc',
    'ucrt/string/_stricoll.cc',
    'ucrt/string/_stricoll_l.cc',
    'ucrt/string/_strlwr.cc',
    'ucrt/string/_strlwr_l.cc',
    'ucrt/string/_strlwr_s.cc',
    'ucrt/string/_strlwr_s_l.cc',
    'ucrt/string/_strncoll.cc',
    'ucrt/string/_strncoll_l.cc',
    -- M14a: the wide collation family.  See ucrt/string/wcs_coll.h.
    'ucrt/string/_wcscoll_l.cc',
    'ucrt/string/_wcsicoll.cc',
    'ucrt/string/_wcsicoll_l.cc',
    'ucrt/string/_wcsncoll.cc',
    'ucrt/string/_wcsncoll_l.cc',
    'ucrt/string/_wcsnicoll.cc',
    'ucrt/string/_wcsnicoll_l.cc',
    'ucrt/string/_wcsxfrm_l.cc',
    'ucrt/string/_strnicmp.cc',
    'ucrt/string/_strnicmp_l.cc',
    'ucrt/string/_strnicoll.cc',
    'ucrt/string/_strnicoll_l.cc',
    'ucrt/string/_strupr.cc',
    'ucrt/string/_strupr_l.cc',
    'ucrt/string/_strupr_s.cc',
    'ucrt/string/_strupr_s_l.cc',
    'ucrt/string/_strxfrm_l.cc',
    'ucrt/string/_tolower.cc',
    'ucrt/string/_tolower_l.cc',
    'ucrt/string/_toupper.cc',
    'ucrt/string/_toupper_l.cc',
    'ucrt/string/_wcsicmp.cc',
    'ucrt/string/_wcsicmp_l.cc',
    'ucrt/string/_wcslwr.cc',
    'ucrt/string/_wcslwr_l.cc',
    'ucrt/string/_wcslwr_s.cc',
    'ucrt/string/_wcslwr_s_l.cc',
    'ucrt/string/_wcsnicmp.cc',
    'ucrt/string/_wcsnicmp_l.cc',
    'ucrt/string/_wcsupr.cc',
    'ucrt/string/_wcsupr_l.cc',
    'ucrt/string/_wcsupr_s.cc',
    'ucrt/string/_wcsupr_s_l.cc',
    'ucrt/string/isalnum.cc',
    'ucrt/string/isalpha.cc',
    'ucrt/string/isblank.cc',
    'ucrt/string/iscntrl.cc',
    'ucrt/string/isdigit.cc',
    'ucrt/string/isgraph.cc',
    'ucrt/string/isleadbyte.cc',
    'ucrt/string/islower.cc',
    'ucrt/string/isprint.cc',
    'ucrt/string/ispunct.cc',
    'ucrt/string/isspace.cc',
    'ucrt/string/isupper.cc',
    'ucrt/string/isxdigit.cc',
    'ucrt/string/mbrlen.cc',
    'ucrt/string/strcoll.cc',
    'ucrt/string/strxfrm.cc',
    -- M14a: the wide collation family (wcscoll/wcsxfrm); see wcs_coll.h.
    'ucrt/string/wcscoll.cc',
    'ucrt/string/wcsxfrm.cc',
    'ucrt/string/tolower.cc',
    'ucrt/string/toupper.cc',
    'ucrt/string/towlower.cc',
    'ucrt/string/towupper.cc',

    'ucrt/time/__daylight.cc',
    'ucrt/time/__dstbias.cc',
    'ucrt/time/__timezone.cc',
    'ucrt/time/__tzname.cc',
    'ucrt/time/_get_tzname.cc',
    'ucrt/time/_strftime_l.cc',
    'ucrt/time/_tzset.cc',
    'ucrt/time/_utime32.cc',
    'ucrt/time/_utime64.cc',
    'ucrt/time/_wcsftime_l.cc',
    'ucrt/time/strftime.cc',
    'ucrt/time/wcsftime.cc',

    'ucrt/runtime/system.cc',
    'ucrt/runtime/__p___argv.cc',
    'ucrt/runtime/__p__pgmptr.cc',
    'ucrt/runtime/_get_pgmptr.cc',
    'ucrt/runtime/_get_initial_narrow_environment.cc',
    'ucrt/runtime/_get_narrow_winmain_command_line.cc',
    'ucrt/runtime/_getdllprocaddr.cc',

    'ucrt/stdio/__stdio_common_vfprintf.cc',
    'ucrt/stdio/__stdio_common_vfprintf_p.cc',
    'ucrt/stdio/__stdio_common_vfprintf_s.cc',
    'ucrt/stdio/__stdio_common_vfscanf.cc',
    'ucrt/stdio/__stdio_common_vfwprintf.cc',
    'ucrt/stdio/__stdio_common_vfwprintf_p.cc',
    'ucrt/stdio/__stdio_common_vfwprintf_s.cc',
    'ucrt/stdio/__stdio_common_vfwscanf.cc',
    'ucrt/stdio/__stdio_common_vsnprintf_s.cc',
    'ucrt/stdio/__stdio_common_vsnwprintf_s.cc',
    'ucrt/stdio/__stdio_common_vsprintf.cc',
    'ucrt/stdio/__stdio_common_vsprintf_p.cc',
    'ucrt/stdio/__stdio_common_vsprintf_s.cc',
    'ucrt/stdio/__stdio_common_vsscanf.cc',
    'ucrt/stdio/__stdio_common_vswprintf.cc',
    'ucrt/stdio/__stdio_common_vswprintf_p.cc',
    'ucrt/stdio/__stdio_common_vswprintf_s.cc',
    'ucrt/stdio/__stdio_common_vswscanf.cc',
    'ucrt/stdio/_close.cc',
    'ucrt/stdio/_fsopen.cc',
    'ucrt/stdio/_get_printf_count_output.cc',
    'ucrt/stdio/_getcwd.cc',
    'ucrt/stdio/_getdcwd.cc',
    'ucrt/stdio/_mktemp.cc',
    'ucrt/stdio/_mktemp_s.cc',
    'ucrt/stdio/_open.cc',
    'ucrt/stdio/_popen.cc',
    'ucrt/stdio/_read.cc',
    'ucrt/stdio/_set_printf_count_output.cc',
    'ucrt/stdio/_sopen.cc',
    'ucrt/stdio/_sopen_dispatch.cc',
    'ucrt/stdio/_sopen_s.cc',
    'ucrt/stdio/_tempnam.cc',
    'ucrt/stdio/_write.cc',
    'ucrt/stdio/close.cc',
    'ucrt/stdio/fflush.cc',
    'ucrt/stdio/fclose.cc',
    'ucrt/stdio/fgetc.cc',
    'ucrt/stdio/fgets.cc',
    'ucrt/stdio/fopen.cc',
    'ucrt/stdio/fopen_s.cc',
    'ucrt/stdio/fputc.cc',
    'ucrt/stdio/fputs.cc',
    'ucrt/stdio/fread.cc',
    'ucrt/stdio/freopen.cc',
    'ucrt/stdio/freopen_s.cc',
    'ucrt/stdio/fwrite.cc',
    'ucrt/stdio/getc.cc',
    'ucrt/stdio/getchar.cc',
    'ucrt/stdio/getcwd.cc',
    'ucrt/stdio/open.cc',
    'ucrt/stdio/popen.cc',
    'ucrt/stdio/putc.cc',
    'ucrt/stdio/puts.cc',
    'ucrt/stdio/read.cc',
    'ucrt/stdio/tmpnam.cc',
    'ucrt/stdio/tmpnam_s.cc',
    'ucrt/stdio/ungetc.cc',
    'ucrt/stdio/write.cc',
  }
end

function ucrt_utf8_startup_deps()
  -- symbols that are referenced by startup object
  -- they should be explicitly added to test target to be chosen
  -- also they depend on some internal declarations not available in old versions
  return {
    'ucrt/environment/__p__environ.cc',
    'ucrt/runtime/__p___argv.cc',
    'ucrt/runtime/_configure_narrow_argv.cc',
    'ucrt/runtime/_initialize_narrow_environment.cc',
  }
end

function ucrt_def_files()
  local result = {
    'def/api-ms-win-crt-convert-l1-1-0.def',
    'def/api-ms-win-crt-environment-l1-1-0.def',
    'def/api-ms-win-crt-locale-l1-1-0.def',
    'def/api-ms-win-crt-runtime-l1-1-0.def',
    'def/api-ms-win-crt-stdio-l1-1-0.def',
    'def/api-ms-win-crt-string-l1-1-0.def',
    'def/api-ms-win-crt-time-l1-1-0.def',
  }

  if is_arch('i386', 'i686') then
    table.insert(result,
      'def/lib32/api-ms-win-crt-filesystem-l1-1-0.def')
  else
    table.insert(result,
      'def/api-ms-win-crt-filesystem-l1-1-0.def')
  end

  return result
end

function add_ucrt_test_links(thunk)
  -- thunk BEFORE mingwex: libmingwex.a defines _strtold/_wcstof/
  -- _wcstold (verified to be the overlay's only name collisions with
  -- mingwex) and would otherwise shadow the M11 thunks; the overlay
  -- preempts nothing else.  (The narrow strtod/strtof/atof names stay
  -- header-inlined to __mingw_strtod in C++ TUs regardless of link
  -- order — plan-3 §3.7 divergence note; the tests drive those paths
  -- through the _l faces, which bind to the overlay.)
  add_linkgroups(
    'catch2', 'stdc++', 'pthread',
    'mingw32', 'gcc', thunk, 'mingwex', 'ucrt')
end

target('alias-short-ucrt')
  on_build(build_short_import_library(ucrt_def_files()))
  set_enabled(has_config('short-alias'))
  set_kind('static')

target('overlay-ucrt')
  add_defines('_UCRT')
  enable_thunk_options()

  if profile_toolchain_utf8() then
    add_deps('utf8-musl.a')
    add_files(table.unpack(ucrt_utf8_files()))
    add_files(table.unpack(ucrt_utf8_startup_deps()))
    set_policy('build.merge_archive', true)
  end

target('alias-short-utf8-ucrt')
  on_build(build_short_import_library(ucrt_def_files()))
  set_enabled(has_config('short-alias') and has_config('u8crt'))
  set_kind('static')

target('overlay-utf8-ucrt')
  add_defines('_UCRT')
  add_files(table.unpack(ucrt_utf8_files()))
  add_files(table.unpack(ucrt_utf8_startup_deps()))
  enable_thunk_options()
  set_enabled(has_config('u8crt'))

target('alias-long-ucrt')
  on_build(build_long_import_library(ucrt_def_files()))
  set_kind('static')
  skip_install()

target('thunk-ucrt-u')
  add_defines('_UCRT')
  add_deps('alias-long-ucrt', 'utf8-musl.a')
  add_files(table.unpack(ucrt_utf8_files()))
  enable_thunk_options()
  merge_win32_alias()
  skip_install()

target('test-ucrt-u')
  add_deps('thunk-ucrt-u')
  add_deps('utf8-musl.a')
  add_files(
    'ucrt/stdio/_getdcwd.test.cc',
    'ucrt/stdio/_open.test.cc',
    'ucrt/stdio/console_channel.test.cc',
    'ucrt/stdio/fopen.test.cc',
    'ucrt/stdio/mktemp_tmpnam_s.test.cc',
    'ucrt/stdio/native_bridge.test.cc',
    'ucrt/stdio/sopen_fsopen.test.cc',
    'ucrt/stdio/stdio_ps.test.cc',
    'ucrt/stdio/stdio_shell.test.cc',
    'ucrt/stdio/vfprintf_engine.test.cc',
    'ucrt/stdio/vsnprintf_vsscanf.test.cc',
    'ucrt/stdio/wfmt_wide.test.cc',
    'ucrt/convert/char16_32.test.cc',
    'ucrt/convert/convert_s.test.cc',
    'ucrt/convert/mbsrtowcs.test.cc',
    'ucrt/convert/mbstowcs.test.cc',
    'ucrt/convert/mbtowc.test.cc',
    'ucrt/convert/mbrtowc.test.cc',
    'ucrt/convert/wcrtomb.test.cc',
    'ucrt/convert/wcstombs.test.cc',
    'ucrt/convert/strtod_narrow.test.cc',
    'ucrt/convert/strtod_wide.test.cc',
    'ucrt/convert/ecvt_atodbl.test.cc',
    'ucrt/conio/conio_channel.test.cc',
    'ucrt/environment/env_s.test.cc',
    'ucrt/filesystem/path_mk_split.test.cc',
    'ucrt/locale/locale_sentinel.test.cc',
    'ucrt/mbstring/ismbc_pred.test.cc',
    'ucrt/mbstring/ismbb_role.test.cc',
    'ucrt/mbstring/mbs_l_delegate.test.cc',
    'ucrt/mbstring/mbs_walk_count.test.cc',
    'ucrt/mbstring/mbs_compare_locate.test.cc',
    'ucrt/mbstring/mbs_copy_case.test.cc',
    'ucrt/mbstring/mbs_char.test.cc',
    'ucrt/mbstring/mbs_data.test.cc',
    'ucrt/process/spawn_process.test.cc',
    'ucrt/string/ctype_fold.test.cc',
    'ucrt/string/ctype_narrow.test.cc',
    'ucrt/string/stricmp_memicmp.test.cc',
    'ucrt/string/strlwr_strupr.test.cc',
    'ucrt/string/strxfrm_coll.test.cc',
    'ucrt/string/wcs_fold.test.cc',
    'ucrt/string/wcs_coll.test.cc',
    'ucrt/time/strftime.test.cc',
    'ucrt/time/tz.test.cc',
    'ucrt/time/utime32.test.cc',
    'ucrt/runtime/getdllprocaddr.test.cc',
    'ucrt/runtime/runtime_f.test.cc')
  add_tests('default')
  add_ucrt_test_links('thunk-ucrt-u')
  enable_test_options()
  skip_install()

target('console-ucrt')
  add_cxflags('-fno-builtin')
  add_defines('_UCRT')
  add_deps('thunk-ucrt-u')
  add_files('test/console.c')
  add_ucrt_test_links('thunk-ucrt-u')
  enable_test_options()
  skip_install()

target('argv-ucrt')
  add_cxxflags('-nostdinc++')
  add_defines(
    'THUNK_LEVEL=' .. ntddi_version(), -- for startup files
    '_UCRT')
  add_deps('thunk-ucrt-u')
  add_files('test/argv.c')
  add_files(table.unpack(ucrt_utf8_startup_deps()))
  add_tests('default', {
    runargs = {"你好", "世界"},
    pass_outputs = "argv[1] = 你好\nargv[2] = 世界\n",
    plain = true,
  })
  add_ucrt_test_links('thunk-ucrt-u')
  enable_test_options()
  skip_install()

-- Spawn target for the M12 process tests.  Built as its own program so
-- the exit code and the echoed argument vector stay observable without
-- the Catch2 runner in the picture; test/spawn_process.test.cc locates
-- it next to __p__pgmptr.
target('spawn-echo-ucrt')
  add_cxflags('-fno-builtin')
  add_defines(
    'THUNK_LEVEL=' .. ntddi_version(),
    '_UCRT')
  add_deps('thunk-ucrt-u')
  add_files('test/spawn_echo.c')
  add_files(table.unpack(ucrt_utf8_startup_deps()))
  add_ucrt_test_links('thunk-ucrt-u')
  enable_test_options()
  skip_install()
