import argparse
import logging
import os
from packaging.version import Version
from pathlib import Path
import shutil
import subprocess
from typing import Optional

from module.debug import shell_here
from module.path import ProjectPaths
from module.profile import BranchProfile
from module.util import XMAKE_ARCH_MAP, add_objects_to_static_lib, ensure, overlayfs_ro
from module.util import cflags_B, configure, make_custom, make_default, make_destdir_install
from module.util import meson_build, meson_config, meson_flags_B, meson_install
from module.util import xmake_build, xmake_config, xmake_install

def _xmake(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    build_dir = paths.src_dir.xmake / 'core'
    xmake_config(build_dir, [
      '-m', 'releasedbg',
      '--plat=mingw',
      f'--arch={XMAKE_ARCH_MAP[ver.arch]}',
    ])
    xmake_build(build_dir, config.jobs)
    xmake_install(build_dir, paths.layer_ABB.xmake, ['cli'])

  license_dir = paths.layer_ABB.xmake / 'share/licenses/xmake'
  ensure(license_dir)
  shutil.copy(paths.src_dir.xmake / 'LICENSE.md', license_dir / 'LICENSE.md')

def build_ABB_xmake(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  _xmake(ver, paths, config)

def _binutils(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',

    paths.layer_AAB.intl / 'usr/local',
  ]):
    build_dir = paths.src_dir.binutils / 'build-ABB'
    ensure(build_dir)

    configure(build_dir, [
      '--prefix=',
      f'--host={ver.target}',
      f'--target={ver.target}',
      f'--build={config.build}',
      # workaround: bfd plugin 'dep' should be built as shared object
      '--enable-shared',
      '--enable-static',
      # features
      '--disable-install-libbfd',
      '--disable-multilib',
      '--enable-nls',
      *cflags_B(
        cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
        optimize_for_size = ver.optimize_for_size,
        lto = not ver.optimize_for_size,
      ),
      f'AR={ver.target}-gcc-ar',
      f'RANLIB={ver.target}-gcc-ranlib',
    ])
    make_default(build_dir, config.jobs)
    make_custom(build_dir, [
      f'DESTDIR={paths.layer_ABB.binutils}',
      # use native layout
      'tooldir=',
      'install',
    ], jobs = 1)

  license_dir = paths.layer_ABB.binutils / 'share/licenses/binutils'
  ensure(license_dir)
  for file in ['COPYING', 'COPYING3', 'COPYING.LIB', 'COPYING3.LIB']:
    shutil.copy(paths.src_dir.binutils / file, license_dir / file)

def _headers(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  build_dir = paths.src_dir.mingw_target / 'mingw-w64-headers' / 'build-ABB'
  ensure(build_dir)
  configure(build_dir, [
    '--prefix=',
    f'--host={ver.target}',
    f'--build={config.build}',
    f'--with-default-msvcrt={ver.default_crt}',
    f'--with-default-win32-winnt=0x{ver.win32_winnt:04X}',
  ])
  make_default(build_dir, config.jobs)
  make_destdir_install(build_dir, paths.layer_ABB.headers)

  include_dir = paths.layer_ABB.headers / 'include'
  for dummy_header in ['pthread_signal.h', 'pthread_time.h', 'pthread_unistd.h']:
    (include_dir / dummy_header).unlink()

  license_dir = paths.layer_ABB.headers / 'share/licenses/headers'
  ensure(license_dir)
  shutil.copy(paths.src_dir.mingw_target / 'COPYING', license_dir / 'COPYING')

def _crt(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    build_dir = paths.src_dir.mingw_target / 'mingw-w64-crt' / 'build-ABB'
    ensure(build_dir)

    multilib_flags = [
      '--enable-lib64' if ver.arch == '64' else '--disable-lib64',
      '--enable-libarm64' if ver.arch == 'arm64' else '--disable-libarm64',
      '--enable-lib32' if ver.arch == '32' else '--disable-lib32',
      '--disable-libarm32',
    ]

    configure(build_dir, [
      '--prefix=',
      f'--host={ver.target}',
      f'--build={config.build}',
      f'--with-default-msvcrt={ver.default_crt}',
      f'--with-default-win32-winnt=0x{max(ver.win32_winnt, 0x0400):04X}',
      *multilib_flags,
      *cflags_B(optimize_for_size = ver.optimize_for_size),
    ])
    make_default(build_dir, config.jobs)
    make_destdir_install(build_dir, paths.layer_ABB.crt)

  license_dir = paths.layer_ABB.crt / 'share/licenses/crt'
  ensure(license_dir)
  shutil.copy(paths.src_dir.mingw_target / 'COPYING', license_dir / 'COPYING')

def _crt_qt(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    build_dir = paths.src_dir.mingw_qt / 'mingw-w64-crt' / 'build-ABB'
    ensure(build_dir)

    multilib_flags = [
      '--enable-lib64' if ver.arch == '64' else '--disable-lib64',
      '--enable-libarm64' if ver.arch == 'arm64' else '--disable-libarm64',
      '--enable-lib32' if ver.arch == '32' else '--disable-lib32',
      '--disable-libarm32',
    ]

    configure(build_dir, [
      '--prefix=',
      f'--host={ver.target}',
      f'--build={config.build}',
      f'--with-default-msvcrt={ver.default_crt}',
      f'--with-default-win32-winnt=0x{max(ver.win32_winnt, 0x0400):04X}',
      *multilib_flags,
      *cflags_B(optimize_for_size = ver.optimize_for_size),
    ])
    make_default(build_dir, config.jobs)
    make_destdir_install(build_dir, paths.layer_ABB.crt_qt)

  license_dir = paths.layer_ABB.crt / 'share/licenses/crt'
  ensure(license_dir)
  shutil.copy(paths.src_dir.mingw_target / 'COPYING', license_dir / 'COPYING')

  license_dir = paths.layer_ABB.crt / 'share/licenses/thunk'
  ensure(license_dir)
  shutil.copy(paths.in_tree_src_dir.thunk / 'LICENSE', license_dir / 'LICENSE')

def _winpthreads(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    build_dir = paths.src_dir.mingw_target / 'mingw-w64-libraries' / 'winpthreads' / 'build-ABB'
    ensure(build_dir)
    configure(build_dir, [
      '--prefix=',
      f'--host={ver.target}',
      f'--build={config.build}',
      '--enable-static',
      '--disable-shared',
      *cflags_B(
        cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
        optimize_for_size = ver.optimize_for_size,
      ),
    ])
    make_default(build_dir, config.jobs)

    # as the basis of gthread interface, it should be considered as part of gcc
    make_destdir_install(build_dir, paths.layer_ABB.gcc)

  license_dir = paths.layer_ABB.gcc / 'share/licenses/winpthreads'
  ensure(license_dir)
  shutil.copy(paths.src_dir.mingw_target / 'mingw-w64-libraries/winpthreads/COPYING', license_dir / 'COPYING')

def _mcfgthread(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    v = Version(ver.mcfgthread.replace('-ga', ''))
    build_dir = 'build-AAB'

    if v >= Version('2.2'):
      cross_file = f'cross/gcc.{ver.target}'
    else:
      cross_file = f'meson.cross.{ver.target}'

    meson_config(
      paths.src_dir.mcfgthread,
      extra_args = [
        '--cross-file', cross_file,
        *meson_flags_B(
          cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
          optimize_for_size = ver.optimize_for_size,
        ),
      ],
      build_dir = build_dir,
    )
    meson_build(
      paths.src_dir.mcfgthread,
      jobs = config.jobs,
      build_dir = build_dir,
      targets = ['mcfgthread:static_library'],
    )

  # as the basis of gthread interface, it should be considered as part of gcc
  full_build_dir = paths.src_dir.mcfgthread / build_dir
  lib_dir = paths.layer_ABB.gcc / 'lib'
  ensure(lib_dir)
  shutil.copy(full_build_dir / 'libmcfgthread.a', lib_dir / 'libmcfgthread.a')

  include_dir = paths.layer_ABB.gcc / 'include' / 'mcfgthread'
  ensure(include_dir)
  header_files = [
    *paths.src_dir.mcfgthread.glob('mcfgthread/*.h'),
    *paths.src_dir.mcfgthread.glob('mcfgthread/*.hpp'),
    full_build_dir / 'version.h',
  ]
  for header_file in header_files:
    shutil.copy(header_file, include_dir / header_file.name)

  license_dir = paths.layer_AAB.gcc / 'share/licenses/mcfgthread'
  ensure(license_dir)
  shutil.copy(paths.src_dir.mcfgthread / 'LICENSE.TXT', license_dir / 'LICENSE.TXT')
  for file in ['gcc-exception-3.1.txt', 'gpl-3.0.txt', 'lgpl-3.0.txt']:
    shutil.copy(paths.src_dir.mcfgthread / 'licenses' / file, license_dir / file)

def _gcc(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',

    paths.layer_AAB.gmp / 'usr/local',
    paths.layer_AAB.mpfr / 'usr/local',
    paths.layer_AAB.mpc / 'usr/local',
    paths.layer_AAB.iconv / 'usr/local',
    paths.layer_AAB.intl / 'usr/local',
  ]):
    v = Version(ver.gcc)
    build_dir = paths.src_dir.gcc / 'build-ABB'
    ensure(build_dir)

    config_flags = []

    if ver.exception == 'dwarf':
      config_flags.append('--disable-sjlj-exceptions')
      config_flags.append('--with-dwarf2')
    if ver.min_os.major >= 6:
      # should also disable because we have it embedded in CRT init objects.
      # but don't do that in case someone use this to detect encoding.
      # multiple manifests seem okay.
      pass
    else:
      config_flags.append('--disable-win32-utf8-manifest')
    if ver.fpmath:
      config_flags.append(f'--with-fpmath={ver.fpmath}')

    configure(build_dir, [
      '--prefix=',
      '--libexecdir=/lib',
      f'--with-gcc-major-version-only',
      f'--target={ver.target}',
      f'--host={ver.target}',
      f'--build={config.build}',
      # prefer static
      '--disable-shared',
      '--enable-static',
      # features
      '--disable-bootstrap',
      '--enable-checking=release',
      '--enable-languages=c,c++',
      '--enable-libgomp',
      '--disable-libmpx',
      '--disable-multilib',
      '--enable-nls',
      f'--enable-threads={ver.thread}',
      '--disable-win32-registry',
      # packages
      f'--with-arch={ver.march}',
      '--without-libcc1',
      '--with-libiconv',
      '--with-tune=generic',
      *config_flags,
      *cflags_B(
        cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
        optimize_for_size = ver.optimize_for_size,
        lto = not ver.optimize_for_size,
      ),
      *cflags_B('_FOR_TARGET',
        cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
        # C++ standard library:
        #   ostream &ostream::operator<<(ostream &(*func)(ostream &))
        #   {
        #     return func(*this);
        #   }
        # is compiled into single instruction:
        #   jmp *rdx
        # hence there's no complete frame info.
        #
        # when debugging, gdb fails to resolve frame info,
        # and will stop here if we want to "step over" it.
        #
        # here we add minimal debug info, so gdb will not be fooled.
        common_extra = ['-g1'],
        optimize_for_size = ver.optimize_for_size,
      ),
      f'AR={ver.target}-gcc-ar',
      f'RANLIB={ver.target}-gcc-ranlib',
    ])
    make_default(build_dir, config.jobs)
    make_destdir_install(build_dir, paths.layer_ABB.gcc)

    # add `print.o` to `libstdc++.a`, allowing `<print>` without `-lstdc++exp`
    # it's okay since we only keep ABI stable in a major version
    if 14 <= v.major < 17:
      add_objects_to_static_lib(f'{ver.target}-ar',
        paths.layer_ABB.gcc / 'lib/libstdc++.a',
        [build_dir / ver.target / 'libstdc++-v3/src/c++23/print.o'],
      )

    # add libatomic to libgcc for convenience
    if ver.march in ['i386', 'i486']:
      libgcc_a = paths.layer_ABB.gcc / 'lib/gcc' / ver.target / str(v.major) / 'libgcc.a'
      atomic_objects = (build_dir / ver.target / 'libatomic').glob('*.o')
      add_objects_to_static_lib(f'{ver.target}-ar', libgcc_a, atomic_objects)

  license_dir = paths.layer_ABB.gcc / 'share/licenses/gcc'
  ensure(license_dir)
  for file in ['COPYING', 'COPYING3', 'COPYING.RUNTIME', 'COPYING.LIB', 'COPYING3.LIB']:
    shutil.copy(paths.src_dir.gcc / file, license_dir / file)

def _gdb(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',

    paths.layer_AAB.expat / 'usr/local',
    paths.layer_AAB.gmp / 'usr/local',
    paths.layer_AAB.mpfr / 'usr/local',
    paths.layer_AAB.mpc / 'usr/local',
    paths.layer_AAB.iconv / 'usr/local',
    paths.layer_AAB.pdcurses / 'usr/local',
    paths.layer_AAB.python / 'usr/local',
  ]):
    v = Version(ver.gdb)
    v_gcc = Version(ver.gcc)
    build_dir = paths.src_dir.gdb / 'build-ABB'
    ensure(build_dir)

    c_extra = []

    # GCC 15 defaults to C23, in which `foo()` means `foo(void)` instead of `foo(...)`.
    if v_gcc.major >= 15 and v < Version('17'):
      c_extra.append('-std=gnu11')

    cflags = cflags_B(
      cpp_extra = [
        f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}',
        '-DPDC_WIDE',
        # workaround: bfd and gnulib disagree about i686 time_t
        '-D__MINGW_USE_VC2005_COMPAT',
      ],
      c_extra = c_extra,
      optimize_for_size = ver.optimize_for_size,
    )

    configure(build_dir, [
      '--prefix=',
      f'--target={ver.target}',
      f'--host={ver.target}',
      f'--build={config.build}',
      # features
      '--disable-install-libbfd',
      # explicitly disable nls to prevent libbfd from installing locale files,
      # which collide with binutils.
      # there is no nls support in gdb.
      '--disable-nls',
      '--enable-tui',
      # packages
      f'--with-python=/usr/local/{ver.target}/python-config.sh',
      f'--with-system-gdbinit=/share/gdb/gdbinit',
      *cflags,
    ])

    # workaround: top level configure does not pass CPPFLAGS to gdbsupport.
    # thus it defaults to _WIN32_WINNT=0x0501, breaking win32 and mcf thread model.
    for item in cflags:
      if item.startswith('CPPFLAGS='):
        os.environ['CPPFLAGS'] = item[len('CPPFLAGS='):]
    make_custom(build_dir, ['configure-host'], config.jobs)
    del os.environ['CPPFLAGS']

    make_default(build_dir, config.jobs)
    make_destdir_install(build_dir, paths.layer_ABB.gdb)

    gdbinit = paths.layer_ABB.gdb / 'share/gdb/gdbinit'
    with open(gdbinit, 'w') as f:
      pass

    shutil.copy(f'/usr/local/{ver.target}/lib/python.zip', paths.layer_ABB.gdb / 'lib/python.zip')
    with open(paths.layer_ABB.gdb / 'bin/gdb._pth', 'w') as f:
      f.write('../lib/python.zip\n')
    with open(gdbinit, 'a') as f:
      f.write('python\n')
      f.write('from libstdcxx.v6.printers import register_libstdcxx_printers\n')
      f.write('register_libstdcxx_printers(None)\n')
      f.write('end\n')

  # collision with binutils
  for info_file in ['bfd.info', 'ctf-spec.info', 'sframe-spec.info']:
    os.unlink(paths.layer_ABB.gdb / 'share/info' / info_file)

  license_dir = paths.layer_ABB.gdb / 'share/licenses/gdb'
  ensure(license_dir)
  for file in ['COPYING', 'COPYING3', 'COPYING.LIB', 'COPYING3.LIB']:
    shutil.copy(paths.src_dir.gdb / file, license_dir / file)

def _gmake(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',

    paths.layer_AAB.iconv / 'usr/local',
    paths.layer_AAB.intl / 'usr/local',
  ]):
    v = Version(ver.make)
    v_gcc = Version(ver.gcc)
    build_dir = paths.src_dir.make / 'build-ABB'
    ensure(build_dir)

    c_extra = []

    # GCC 15 defaults to C23, in which `foo()` means `foo(void)` instead of `foo(...)`.
    if v_gcc.major >= 15 and v < Version('4.5'):
      c_extra.append('-std=gnu11')

    configure(build_dir, [
      '--prefix=',
      '--program-prefix=mingw32-',
      f'--host={ver.target}',
      f'--build={config.build}',
      '--enable-nls',
      *cflags_B(
        cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
        c_extra = c_extra,
        optimize_for_size = ver.optimize_for_size,
      ),
    ])
    make_default(build_dir, config.jobs)
    make_destdir_install(build_dir, paths.layer_ABB.make)

  license_dir = paths.layer_ABB.make / 'share/licenses/make'
  ensure(license_dir)
  shutil.copy(paths.src_dir.make / 'COPYING', license_dir / 'COPYING')

def _pkgconf(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  with overlayfs_ro('/usr/local', [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.crt / 'usr/local',
  ]):
    build_dir = 'build-ABB'

    meson_config(
      paths.src_dir.pkgconf,
      extra_args = [
        '--cross-file', paths.meson_cross_file,
        '--prefix', '/',
        '--default-library', 'static',
        '--prefer-static',
        '-Dtests=disabled',
        *meson_flags_B(
          cpp_extra = [f'-D_WIN32_WINNT=0x{ver.min_winnt:04X}'],
          optimize_for_size = ver.optimize_for_size,
        ),
      ],
      build_dir = build_dir,
    )
    meson_build(
      paths.src_dir.pkgconf,
      jobs = config.jobs,
      build_dir = build_dir,
    )
    meson_install(
      paths.src_dir.pkgconf,
      build_dir = build_dir,
      destdir = paths.layer_ABB.pkgconf,
    )

  license_dir = paths.layer_ABB.pkgconf / 'share/licenses/pkgconf'
  ensure(license_dir)
  shutil.copy(paths.src_dir.pkgconf / 'COPYING', license_dir / 'COPYING')

def build_ABB_toolchain(ver: BranchProfile, paths: ProjectPaths, config: argparse.Namespace):
  _binutils(ver, paths, config)

  _headers(ver, paths, config)

  _crt(ver, paths, config)

  if config.qt:
    _crt_qt(ver, paths, config)

  _winpthreads(ver, paths, config)

  if ver.thread == 'mcf':
    _mcfgthread(ver, paths, config)

  _gcc(ver, paths, config)

  _gdb(ver, paths, config)

  _gmake(ver, paths, config)

  _pkgconf(ver, paths, config)
