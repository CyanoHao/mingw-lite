from hashlib import sha256
import logging
import os
from packaging.version import Version
from pathlib import Path
import re
import shutil
import subprocess
from urllib.error import URLError
from urllib.request import urlopen

from module.fetch import validate_and_download, check_and_extract, check_and_sync, patch, patch_done
from module.path import ProjectPaths
from module.profile import BranchProfile

def _autoreconf(path: Path):
  subprocess.run(
    ['autoreconf', '-fi'],
    cwd = path,
    check = True,
  )

def _automake(path: Path):
  subprocess.run(
    ['automake'],
    cwd = path,
    check = True,
  )

def _binutils(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/binutils/{paths.src_arx.binutils.name}'
  validate_and_download(paths.src_arx.binutils, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.binutils, paths.src_arx.binutils):
    v = Version(ver.binutils)

    # Fix path corruption
    if v >= Version('2.43'):
      patch(paths.src_dir.binutils, paths.patch_dir / 'binutils' / 'fix-path-corruption_2.43.patch')
    else:
      patch(paths.src_dir.binutils, paths.patch_dir / 'binutils' / 'fix-path-corruption_2.41.patch')

    # Don't optimize out libtool wrapper magic
    patch(paths.src_dir.binutils, paths.patch_dir / 'binutils' / 'dont-optimize-out-libtool-wrapper-magic.patch')

    patch_done(paths.src_dir.binutils)

def _expat(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  v = Version(ver.expat)
  tag = f'R_{v.major}_{v.minor}_{v.micro}'
  url = f'https://github.com/libexpat/libexpat/releases/download/{tag}/{paths.src_arx.expat.name}'
  validate_and_download(paths.src_arx.expat, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.expat, paths.src_arx.expat)
  patch_done(paths.src_dir.expat)

def _gcc(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  v = Version(ver.gcc)

  is_snapshot = re.search(r'-\d{8}$', ver.gcc)
  if is_snapshot:
    url = f'https://gcc.gnu.org/pub/gcc/snapshots/{ver.gcc}/{paths.src_arx.gcc.name}'
  else:
    url = f'https://ftpmirror.gnu.org/gnu/gcc/gcc-{ver.gcc}/{paths.src_arx.gcc.name}'

  validate_and_download(paths.src_arx.gcc, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.gcc, paths.src_arx.gcc):
    # Fix make variable
    # - gcc 12 use `override CFLAGS +=` to handle PGO build, which breaks workaround for ucrt `access`
    if v.major >= 14:
      patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'fix-make-variable_14.patch')
    else:
      patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'fix-make-variable_12.patch')

    # Fix VT sequence
    patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'fix-vt-seq.patch')

    # Fix UCRT pipe
    patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'fix-ucrt-pipe.patch')

    # Fix libcpp setlocale
    # libcpp defines `setlocale` if `HAVE_SETLOCALE` not defined, but its configure.ac does not check `setlocale` at all
    patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'fix-libcpp-setlocale.patch')

    # Disable vectorized lexer
    if ver.min_os.major < 5:
      if v.major >= 15:
        patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'disable-vectorized-lexer_15.patch')
      else:
        patch(paths.src_dir.gcc, paths.patch_dir / 'gcc' / 'disable-vectorized-lexer_13.patch')

    # Parser-friendly diagnostics
    po_dir = paths.src_dir.gcc / 'gcc' / 'po'
    po_files = list(po_dir.glob('*.po'))
    subprocess.run([
      'sed',
      '-i', '-E',
      '/^msgid "(error|warning): "/,+1 d',
      *po_files
    ], check = True)

    patch_done(paths.src_dir.gcc)

def _gdb(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/gdb/{paths.src_arx.gdb.name}'
  validate_and_download(paths.src_arx.gdb, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.gdb, paths.src_arx.gdb):
    v = Version(ver.gdb)

    # Fix thread
    if v.major >= 16:
      patch(paths.src_dir.gdb, paths.patch_dir / 'gdb' / 'fix-thread_16.patch')
    else:
      patch(paths.src_dir.gdb, paths.patch_dir / 'gdb' / 'fix-thread_14.patch')

    # Fix iconv 'CP65001'
    patch(paths.src_dir.gdb, paths.patch_dir / 'gdb' / 'fix-iconv-cp65001.patch')

    # Fix pythondir
    patch(paths.src_dir.gdb, paths.patch_dir / 'gdb' / 'fix-pythondir.patch')

    # Fix ui-style regex init
    if v.major >= 16:
      patch(paths.src_dir.gdb, paths.patch_dir / 'gdb' / 'fix-ui-style-regex-init.patch')

    patch_done(paths.src_dir.gdb)

def _gmp(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/gmp/{paths.src_arx.gmp.name}'
  validate_and_download(paths.src_arx.gmp, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.gmp, paths.src_arx.gmp)
  patch_done(paths.src_dir.gmp)

def _iconv(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/libiconv/{paths.src_arx.iconv.name}'
  validate_and_download(paths.src_arx.iconv, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.iconv, paths.src_arx.iconv)
  patch_done(paths.src_dir.iconv)

def _intl(ver: BranchProfile, paths: ProjectPaths):
  shutil.copytree(
    paths.in_tree_src_tree.intl,
    paths.in_tree_src_dir.intl,
    ignore = shutil.ignore_patterns(
      '.cache',
      '.vscode',
      '.xmake',
      'build',
      'pkg',
      '*.mo',
    ),
    dirs_exist_ok = True,
  )

def _make(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/make/{paths.src_arx.make.name}'
  validate_and_download(paths.src_arx.make, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.make, paths.src_arx.make)
  patch_done(paths.src_dir.make)

def _mcfgthread(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://github.com/lhmouse/mcfgthread/archive/refs/tags/v{ver.mcfgthread}.tar.gz'
  validate_and_download(paths.src_arx.mcfgthread, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.mcfgthread, paths.src_arx.mcfgthread)
  patch_done(paths.src_dir.mcfgthread)

def _mingw(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://downloads.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/{paths.src_arx.mingw.name}'
  validate_and_download(paths.src_arx.mingw, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.mingw, paths.src_arx.mingw):
    v = Version(ver.mingw)

    # CRT: Fix x64 wassert
    if ver.min_os.major < 6 and v.major < 13:
      if v.major >= 12:
        patch(paths.src_dir.mingw, paths.patch_dir / 'crt' / 'fix-x64-wassert_12.patch')
      else:
        patch(paths.src_dir.mingw, paths.patch_dir / 'crt' / 'fix-x64-wassert_11.patch')

    # CRT: Fix x64 difftime64
    if ver.min_os.major < 6 and v.major == 11:
      patch(paths.src_dir.mingw, paths.patch_dir / 'crt' / 'fix-x64-difftime64.patch')

    # CRT: Fix i386 strtoi64, strtoui64
    if ver.min_os < Version('5.1') and v.major == 11:
      patch(paths.src_dir.mingw, paths.patch_dir / 'crt' / 'fix-i386-strtoi64-strtoui64.patch')

    # Winpthreads: Fix linkage
    if v.major == 13:
      patch(paths.src_dir.mingw, paths.patch_dir / 'winpthreads' / 'fix-linkage.patch')

    patch_done(paths.src_dir.mingw)

def _mingw_host(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  if download_only:
    return

  if check_and_sync(paths.src_dir.mingw_host, paths.src_dir.mingw):
    # CRT: Add mingw thunks
    subprocess.run([
      './patch.py',
      paths.src_dir.mingw_host,
      '-a', ver.arch,
      '--level', 'toolchain',
      '--nt-ver', str(ver.min_os),
    ], cwd = paths.in_tree_src_tree.thunk, check = True)

    _autoreconf(paths.src_dir.mingw_host / 'mingw-w64-crt')
    _automake(paths.src_dir.mingw_host / 'mingw-w64-crt')

    patch_done(paths.src_dir.mingw_host)

def _mingw_target(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  if download_only:
    return

  if check_and_sync(paths.src_dir.mingw_target, paths.src_dir.mingw):
    # CRT: Add mingw thunks
    subprocess.run([
      './patch.py',
      paths.src_dir.mingw_target,
      '-a', ver.arch,
      '--level', 'core',
      '--nt-ver', str(ver.min_os),
    ], cwd = paths.in_tree_src_tree.thunk, check = True)

    _autoreconf(paths.src_dir.mingw_target / 'mingw-w64-crt')
    _automake(paths.src_dir.mingw_target / 'mingw-w64-crt')

    patch_done(paths.src_dir.mingw_target)

def _mingw_qt(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  if download_only:
    return

  if check_and_sync(paths.src_dir.mingw_qt, paths.src_dir.mingw):
    # CRT: Add mingw thunks
    subprocess.run([
      './patch.py',
      paths.src_dir.mingw_qt,
      '-a', ver.arch,
      '--level', 'qt',
      '--nt-ver', str(ver.min_os),
    ], cwd = paths.in_tree_src_tree.thunk, check = True)

    _autoreconf(paths.src_dir.mingw_qt / 'mingw-w64-crt')
    _automake(paths.src_dir.mingw_qt / 'mingw-w64-crt')

    patch_done(paths.src_dir.mingw_qt)

def _mpc(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/mpc/{paths.src_arx.mpc.name}'
  validate_and_download(paths.src_arx.mpc, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.mpc, paths.src_arx.mpc)
  patch_done(paths.src_dir.mpc)

def _mpfr(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://ftpmirror.gnu.org/gnu/mpfr/{paths.src_arx.mpfr.name}'
  validate_and_download(paths.src_arx.mpfr, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.mpfr, paths.src_arx.mpfr)
  patch_done(paths.src_dir.mpfr)

def _pdcurses(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://github.com/wmcbrine/PDCurses/archive/refs/tags/{ver.pdcurses}.tar.gz'
  validate_and_download(paths.src_arx.pdcurses, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.pdcurses, paths.src_arx.pdcurses)
  patch_done(paths.src_dir.pdcurses)

def _pkgconf(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://github.com/pkgconf/pkgconf/archive/refs/tags/pkgconf-{ver.pkgconf}.tar.gz'
  validate_and_download(paths.src_arx.pkgconf, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.pkgconf, paths.src_arx.pkgconf):
    ver = Version(ver.pkgconf)

    # Build for static toolchain
    if ver >= Version('2.5.0'):
      patch(paths.src_dir.pkgconf, paths.patch_dir / 'pkgconf/static-toolchain_2.5.patch')
    else:
      patch(paths.src_dir.pkgconf, paths.patch_dir / 'pkgconf/static-toolchain_2.1.patch')

    patch_done(paths.src_dir.pkgconf)

def _python(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://www.python.org/ftp/python/{ver.python}/{paths.src_arx.python.name}'
  validate_and_download(paths.src_arx.python, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.python, paths.src_arx.python):
    ver = Version(ver.python)

    # Fix thread touch last error
    # https://github.com/python/cpython/pull/104531
    if ver >= Version('3.12') and ver < Version('3.13'):
      patch(paths.src_dir.python, paths.patch_dir / 'python' / 'fix-thread-touch-last-error_3.12.patch')

    # Alternative build system
    os.symlink(paths.src_dir.z, paths.src_dir.python / 'zlib', target_is_directory = True)
    if ver >= Version('3.13'):
      shutil.copy(paths.patch_dir / 'python' / 'xmake_3.13.lua', paths.src_dir.python / 'xmake.lua')
      patch(paths.src_dir.python, paths.patch_dir / 'python' / 'fix-mingw-build_3.13.patch')
    else:
      shutil.copy(paths.patch_dir / 'python' / 'xmake_3.12.lua', paths.src_dir.python / 'xmake.lua')
      patch(paths.src_dir.python, paths.patch_dir / 'python' / 'fix-mingw-build_3.12.patch')
    shutil.copy(paths.patch_dir / 'python' / 'python-config.sh', paths.src_dir.python / 'python-config.sh')

    patch_done(paths.src_dir.python)

def _xmake(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  release_name = paths.src_arx.xmake.name.replace('xmake-', 'xmake-v')
  url = f'https://github.com/xmake-io/xmake/releases/download/v{ver.xmake}/{release_name}'
  validate_and_download(paths.src_arx.xmake, url)
  if download_only:
    return

  if check_and_extract(paths.src_dir.xmake, paths.src_arx.xmake):
    tbox = paths.src_dir.xmake / 'core/src/tbox/tbox'

    # disable werror
    xmake_lua = paths.src_dir.xmake / 'core' / 'xmake.lua'
    with open(xmake_lua, 'r') as f:
      xmake_lua_content = f.readlines()
    with open(xmake_lua, 'w') as f:
      for line in xmake_lua_content:
        if line.startswith('set_warnings'):
          f.write('set_warnings("all")\n')
        else:
          f.write(line)

    # Tbox: ignore process group
    patch(tbox, paths.patch_dir / 'xmake' / 'tbox-ignore-process-group.patch')

    patch_done(paths.src_dir.xmake)

def _z(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  url = f'https://zlib.net/fossils/{paths.src_arx.z.name}'
  validate_and_download(paths.src_arx.z, url)
  if download_only:
    return

  check_and_extract(paths.src_dir.z, paths.src_arx.z)
  patch_done(paths.src_dir.z)

def prepare_source(ver: BranchProfile, paths: ProjectPaths, download_only: bool):
  _binutils(ver, paths, download_only)
  _expat(ver, paths, download_only)
  _gcc(ver, paths, download_only)
  _gdb(ver, paths, download_only)
  _gmp(ver, paths, download_only)
  _iconv(ver, paths, download_only)
  _intl(ver, paths)
  _make(ver, paths, download_only)
  if ver.thread == 'mcf':
    _mcfgthread(ver, paths, download_only)
  _mingw(ver, paths, download_only)
  _mingw_host(ver, paths, download_only)
  _mingw_target(ver, paths, download_only)
  _mingw_qt(ver, paths, download_only)
  _mpc(ver, paths, download_only)
  _mpfr(ver, paths, download_only)
  _pdcurses(ver, paths, download_only)
  _pkgconf(ver, paths, download_only)
  _python(ver, paths, download_only)
  _xmake(ver, paths, download_only)
  _z(ver, paths, download_only)
